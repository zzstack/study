const { init } = require('@sentry/electron');

init({
  // TODO: Replace with your project's DSN
  dsn: 'http://37f8a2ee37c0409d8970bc7559c7c7e4@sentry.io/277345',
  //dsn: 'https://232b7db3267840ebb508fda8ea757949@ee.bytedance.net/sentry/128',
  //dsn: 'http://232b7db3267840ebb508fda8ea757949:efe8014ed15d401d8e9d1226c66f235f@ee-sentry.byted.org/128',
  //dsn: 'https://232b7db3267840ebb508fda8ea757949@ee-sentry.byted.org/128',
  //dsn: 'http://1926a003ecc445569990a66e9cc3c803@frontend-sentry.ee-dns.top//3',
  //dsn: 'https://290b4046788f4284ae2e6d3436f1ba42@ee-sentry.byted.org/302',
});

/*
const Sentry = require('@sentry/browser');

function initSentry() {
  console.log("initSentry:" + Sentry);
  let DSN = 'http://232b7db3267840ebb508fda8ea757949:efe8014ed15d401d8e9d1226c66f235f@ee-sentry.byted.org/128'; //  pc-client-development test project

  Sentry.init({
    // Client's DSN.
    dsn: DSN,
  });
}

initSentry();
*/
