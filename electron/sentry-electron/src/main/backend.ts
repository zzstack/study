import { app, crashReporter, ipcMain } from 'electron';
import { join } from 'path';

import { Breadcrumb, SentryEvent, SentryResponse, Severity, Status } from '@sentry/types';

import {
  addBreadcrumb,
  BaseBackend,
  captureEvent,
  captureMessage,
  configureScope,
  Dsn,
  Scope,
  SentryError,
} from '@sentry/core';
import { getCurrentHub, NodeBackend } from '@sentry/node';
import { forget } from '@sentry/utils/async';
import { Store } from '@sentry/utils/store';

import { CommonBackend, ElectronOptions, IPC_CRUMB, IPC_EVENT, IPC_PING, IPC_SCOPE } from '../common';
import { captureMinidump } from '../sdk';
import { normalizeUrl } from './normalize';
import { MinidumpUploader } from './uploader';

/** Patch to access internal CrashReporter functionality. */
interface CrashReporterExt {
  getCrashesDirectory(): string;
}

/** Gets the path to the Sentry cache directory. */
function getCachePath(): string {
  return join(app.getPath('userData'), 'sentry');
}

/**
 * Retruns a promise that resolves when app is ready.
 */
export async function isAppReady(): Promise<boolean> {
  return (
    app.isReady() ||
    new Promise<boolean>(resolve => {
      app.once('ready', resolve);
    })
  );
}

/** Backend implementation for Electron renderer backends. */
export class MainBackend extends BaseBackend<ElectronOptions> implements CommonBackend {
  /** The inner SDK used to record Node events. */
  private readonly inner: NodeBackend;

  /** Store to persist context information beyond application crashes. */
  private readonly scopeStore: Store<Scope>;

  /** Uploader for minidump files. */
  private uploader?: MinidumpUploader;

  /** Creates a new Electron backend instance. */
  public constructor(options: ElectronOptions) {
    super(options);
    this.inner = new NodeBackend(options);
    this.scopeStore = new Store<Scope>(getCachePath(), 'scope', new Scope());
  }

  /**
   * @inheritDoc
   */
  public install(): boolean {
    let success = true;

    // We refill the scope here to not have an empty one
    configureScope(scope => {
      // tslint:disable:no-unsafe-any
      const loadedScope = Scope.clone(this.scopeStore.get()) as any;

      if (loadedScope.user) {
        scope.setUser(loadedScope.user);
      }
      if (loadedScope.tags) {
        Object.keys(loadedScope.tags).forEach(key => {
          scope.setTag(key, loadedScope.tags[key]);
        });
      }
      if (loadedScope.extra) {
        Object.keys(loadedScope.extra).forEach(key => {
          scope.setExtra(key, loadedScope.extra[key]);
        });
      }
      if (loadedScope.breadcrumbs) {
        loadedScope.breadcrumbs.forEach((crumb: any) => {
          scope.addBreadcrumb(crumb);
        });
      }
      // tslint:enable:no-unsafe-any
    });

    if (this.isNativeEnabled()) {
      success = this.installNativeHandler() && success;
    }

    this.installIPC();

    return success;
  }

  /**
   * @inheritDoc
   */
  public async eventFromException(exception: any): Promise<SentryEvent> {
    return this.inner.eventFromException(exception);
  }

  /**
   * @inheritDoc
   */
  public async eventFromMessage(message: string): Promise<SentryEvent> {
    return this.inner.eventFromMessage(message);
  }

  /**
   * @inheritDoc
   */
  public async sendEvent(event: SentryEvent): Promise<SentryResponse> {
    await isAppReady();
    return this.inner.sendEvent(event);
  }

  /**
   * Uploads the given minidump and attaches event information.
   *
   * @param path A relative or absolute path to the minidump file.
   * @param event Optional event information to add to the minidump request.
   * @returns A promise that resolves to the status code of the request.
   */
  public async uploadMinidump(path: string, event: SentryEvent = {}): Promise<SentryResponse> {
    if (this.uploader) {
      return this.uploader.uploadMinidump({ path, event });
    }
    return { status: Status.Success };
  }

  /**
   * @inheritDoc
   */
  public storeScope(scope: Scope): void {
    const cloned = Scope.clone(scope);
    (cloned as any).eventProcessors = [];
    // tslint:disable-next-line:no-object-literal-type-assertion
    this.scopeStore.update((current: Scope) => ({ ...current, ...cloned } as Scope));
  }

  /** Returns whether native reports are enabled. */
  private isNativeEnabled(): boolean {
    // Mac AppStore builds cannot run the crash reporter due to the sandboxing
    // requirements. In this case, we prevent enabling native crashes entirely.
    // https://electronjs.org/docs/tutorial/mac-app-store-submission-guide#limitations-of-mas-build
    if (process.mas) {
      return false;
    }

    return this.options.enableNative !== false;
  }

  /** Activates the Electron CrashReporter. */
  private installNativeHandler(): boolean {
    // We are only called by the frontend if the SDK is enabled and a valid DSN
    // has been configured. If no DSN is present, this indicates a programming
    // error.
    const dsnString = this.options.dsn;
    if (!dsnString) {
      throw new SentryError('Invariant exception: install() must not be called when disabled');
    }

    const dsn = new Dsn(dsnString);

    // We will manually submit errors, but CrashReporter requires a submitURL in
    // some versions. Also, provide a productName and companyName, which we will
    // add manually to the event's context during submission.
    crashReporter.start({
      companyName: '',
      ignoreSystemCrashHandler: true,
      productName: app.getName(),
      submitURL: MinidumpUploader.minidumpUrlFromDsn(dsn),
      uploadToServer: false,
    });

    // The crashReporter has an undocumented method to retrieve the directory
    // it uses to store minidumps in. The structure in this directory depends
    // on the crash library being used (Crashpad or Breakpad).
    const reporter: CrashReporterExt = crashReporter as any;
    const crashesDirectory = reporter.getCrashesDirectory();

    this.uploader = new MinidumpUploader(dsn, crashesDirectory, getCachePath());

    // Flush already cached minidumps from the queue.
    forget(this.uploader.flushQueue());

    // Start to submit recent minidump crashes. This will load breadcrumbs and
    // context information that was cached on disk prior to the crash.
    forget(this.sendNativeCrashes({}));

    // Every time a subprocess or renderer crashes, start sending minidumps
    // right away.
    app.on('web-contents-created', (_, contents) => {
      contents.on('crashed', async () => {
        try {
          await this.sendNativeCrashes(this.getRendererExtra(contents));
        } catch (e) {
          console.error(e);
        }

        addBreadcrumb({
          category: 'exception',
          level: Severity.Critical,
          message: 'Renderer Crashed',
          timestamp: new Date().getTime() / 1000,
        });
      });

      if (this.options.enableUnresponsive !== false) {
        contents.on('unresponsive', () => {
          captureMessage('BrowserWindow Unresponsive');
        });
      }
    });

    return true;
  }

  /** Installs IPC handlers to receive events and metadata from renderers. */
  private installIPC(): void {
    ipcMain.on(IPC_PING, (event: Electron.Event) => {
      event.sender.send(IPC_PING);
    });

    ipcMain.on(IPC_CRUMB, (_: any, crumb: Breadcrumb) => {
      addBreadcrumb(crumb);
    });

    ipcMain.on(IPC_EVENT, (ipc: Electron.Event, event: SentryEvent) => {
      event.extra = {
        ...this.getRendererExtra(ipc.sender),
        ...event.extra,
      };
      captureEvent(event);
    });

    ipcMain.on(IPC_SCOPE, (_: any, rendererScope: Scope) => {
      // tslint:disable:no-unsafe-any
      const sentScope = Scope.clone(rendererScope) as any;
      configureScope(scope => {
        if (sentScope.user) {
          scope.setUser(sentScope.user);
        }
        if (sentScope.tags) {
          Object.keys(sentScope.tags).forEach(key => {
            scope.setTag(key, sentScope.tags[key]);
          });
        }
        if (sentScope.extra) {
          Object.keys(sentScope.extra).forEach(key => {
            scope.setExtra(key, sentScope.extra[key]);
          });
        }
      });
      // tslint:enable:no-unsafe-any
    });
  }

  /** Loads new native crashes from disk and sends them to Sentry. */
  private async sendNativeCrashes(extra: object): Promise<void> {
    // Whenever we are called, assume that the crashes we are going to load down
    // below have occurred recently. This means, we can use the same event data
    // for all minidumps that we load now. There are two conditions:
    //
    //  1. The application crashed and we are just starting up. The stored
    //     breadcrumbs and context reflect the state during the application
    //     crash.
    //
    //  2. A renderer process crashed recently and we have just been notified
    //     about it. Just use the breadcrumbs and context information we have
    //     right now and hope that the delay was not too long.

    const uploader = this.uploader;
    if (uploader === undefined) {
      throw new SentryError('Invariant violation: Native crashes not enabled');
    }

    const currentCloned = Scope.clone(getCurrentHub().getScope());
    const fetchedScope = this.scopeStore.get();
    const storedScope = Scope.clone(fetchedScope);
    let event: SentryEvent | null = { extra };
    event = await storedScope.applyToEvent(event);
    event = event && (await currentCloned.applyToEvent(event));
    const paths = await uploader.getNewMinidumps();
    paths.map(path => {
      captureMinidump(path, { ...event });
    });
  }

  /** Returns extra information from a renderer's web contents. */
  private getRendererExtra(contents: Electron.WebContents): { [key: string]: any } {
    const customName = this.options.getRendererName && this.options.getRendererName(contents);

    return {
      crashed_process: customName || `renderer[${contents.id}]`,
      crashed_url: normalizeUrl(contents.getURL()),
    };
  }
}
