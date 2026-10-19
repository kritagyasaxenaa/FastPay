# FastPay Android (React Native)

This React Native app is an Android client for FastPay's simulated bank ledger. Payments move play money only; it is a hosted test demo, not a real bank or production payment service.

## Requirements

- Node.js 22.13 or newer
- Android Studio with Android SDK and an Android Emulator or device
- Python dependencies for the demo API; see `../server/README.md`

## Run on the Android Emulator

1. Start the API on the computer at `127.0.0.1:8000` using the server README's demo-only instructions.
2. From this folder, install dependencies once with `npm install`.
3. Start Metro and open the emulator with `npm run android`.

The app starts with `EXPO_PUBLIC_API_URL` if set, otherwise the emulator bridge URL `http://10.0.2.2:8000`. The Android network security policy allows HTTP only to that address; other hosts must use HTTPS. The server URL can be changed from the top-right settings button.

## Test on a phone

Download [FastPay-Phone-Demo-test.apk](FastPay-Phone-Demo-test.apk) and install it on an arm64 Android phone. The build already points at the hosted demo server, so no setup or local Wi-Fi server is required. Android may ask you to allow installation from your browser or file manager. Open FastPay Demo and use the pre-seeded `alice@fastpay` and `bob@fastpay` accounts.

The public demo supports only health checks, those seeded fake balances, and simulated transfers up to INR 50. It is rate-limited and stores its test ledger ephemerally, so balances may reset when the service restarts. Do not enter real credentials or expect real payments.

## Build an APK

```bash
npm install
npm run prebuild:android
cd android
./gradlew assembleDebug
```

The debug APK is created at `android/app/build/outputs/apk/debug/app-debug.apk`. To build a release-variant test APK for the hosted demo, set `EXPO_PUBLIC_API_URL=https://fastpay-demo-api-production.up.railway.app` before running `./gradlew assembleRelease`. The generated `android/` project is intentionally ignored by Git; Expo config and the network security plugin generate it reproducibly. The shared phone-test APK is signed with a development key and is for testing only.

## Project layout

- `App.tsx` — native React Native payment screen and local interaction state.
- `src/api.ts` — bounded API calls and demo response validation.
- `plugins/withDemoNetworkSecurity.js` — restricts cleartext networking to the Android emulator's host bridge.
- `app.json` — app identity and native Android configuration.

## Server URL

Hosted test API: `https://fastpay-demo-api-production.up.railway.app`. Set `EXPO_PUBLIC_API_URL` at bundle time to choose another default HTTPS server URL. For the local Android Emulator, leave it unset to use `http://10.0.2.2:8000`.
