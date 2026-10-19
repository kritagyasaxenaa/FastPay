# FastPay Android (React Native)

This React Native app is an Android client for FastPay's local simulated ledger. Payments move play money only. It is not a bank app and is not production-ready.

## Requirements

- Node.js 22.13 or newer
- Android Studio with Android SDK and an Android Emulator or device
- Python dependencies for the demo API; see `../server/README.md`

## Run on the Android Emulator

1. Start the API on the computer at `127.0.0.1:8000` using the server README's demo-only instructions.
2. From this folder, install dependencies once with `npm install`.
3. Start Metro and open the emulator with `npm run android`.

The app starts with the emulator bridge URL `http://10.0.2.2:8000`. The Android network security policy allows HTTP only to that address; other hosts must use HTTPS. The server URL can be changed from the top-right settings button.

## Build a debug APK

```bash
npm install
npm run prebuild:android
cd android
./gradlew assembleDebug
```

The APK is created at `android/app/build/outputs/apk/debug/app-debug.apk`. The generated `android/` project is intentionally ignored by Git; Expo config and the network security plugin generate it reproducibly.

## Project layout

- `App.tsx` — native React Native payment screen and local interaction state.
- `src/api.ts` — bounded API calls and demo response validation.
- `plugins/withDemoNetworkSecurity.js` — restricts cleartext networking to the Android emulator's host bridge.
- `app.json` — app identity and native Android configuration.

## Server URL

Set `EXPO_PUBLIC_API_URL` at bundle time to choose a default HTTPS server URL. For the local Android Emulator, leave it unset to use `http://10.0.2.2:8000`. The simulator must remain private and must not receive real credentials or funds.
