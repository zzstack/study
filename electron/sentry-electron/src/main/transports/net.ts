import { PromiseBuffer, SentryError } from '@sentry/core';
import { Transports } from '@sentry/node';
import { SentryResponse, Status, TransportOptions } from '@sentry/types';
import { net } from 'electron';
import { isAppReady } from '../backend';

/** Using net module of electron */
export class NetTransport extends Transports.BaseTransport {
  /** A simple buffer holding all requests. */
  protected readonly buffer: PromiseBuffer<SentryResponse> = new PromiseBuffer(30);

  /** Create a new instance and set this.agent */
  public constructor(public options: TransportOptions) {
    super(options);
  }

  /**
   * @inheritDoc
   */
  public async sendEvent(body: string): Promise<SentryResponse> {
    await isAppReady();
    return this.buffer.add(
      new Promise<SentryResponse>((resolve, reject) => {
        const req = net.request(this.getRequestOptions());
        req.on('error', reject);
        req.on('response', (res: Electron.IncomingMessage) => {
          if (res.statusCode && res.statusCode >= 200 && res.statusCode < 300) {
            resolve({
              status: Status.fromHttpCode(res.statusCode),
            });
          } else {
            // tslint:disable:no-unsafe-any
            if (res.headers && res.headers['x-sentry-error']) {
              const reason = res.headers['x-sentry-error'];
              // tslint:enable:no-unsafe-any
              reject(new SentryError(`HTTP Error (${res.statusCode}): ${reason}`));
            } else {
              reject(new SentryError(`HTTP Error (${res.statusCode})`));
            }
          }
          // force the socket to drain
          res.on('data', () => {
            // Drain
          });
          res.on('end', () => {
            // Drain
          });
        });
        req.write(body);
        req.end();
      }),
    );
  }
}
