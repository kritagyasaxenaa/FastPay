# FastPay - Payment Flow Prototype

A Qt-based C++ prototype exploring online and offline payment flows with ultrasound-based proximity discovery. It is not a real payment product and must not be used to move money or store production credentials.

## Production status

**Not production-ready.** The Android app contains demo transaction flows and placeholder cryptography. The server's simulated balances are play money, users are not authenticated, and offline signatures are not verified. The production-named API disables all `/api/v1/` operations by default except `/api/v1/health`. Setting `FASTPAY_DEMO_MODE=true` enables the simulator for isolated demonstrations only. Never set that flag on a public deployment.

The current Android app uses Qt Widgets. It is a C++ app that can be packaged for Android, but it does not use Android's native UI toolkit. A native Android UI would require a Kotlin/Java layer and a deliberate JNI boundary to the C++ logic.

## Simulated bank demo

For local demonstrations, run the API with `FASTPAY_DEMO_MODE=true`. It seeds test accounts with INR 1,000.00 and simulates transfers by debiting and crediting the in-app ledger. The transfer API supports retries without duplicate debits and exposes demo balances at `/api/v1/demo/bank-accounts/{account_id}`. It is play money only; it does not connect to banks, UPI, or other payment rails. Keep demo mode off on public deployments.

## Build the Android APK

### Step 1: Start the local demo API

Follow the local demo steps in [`server/README.md`](server/README.md). The release app does not contain a production service URL.

### Step 2: Build APK

Use Qt Creator:
1. Open `FastPayQt.pro` or `CMakeLists.txt`
2. Select Android kit
3. Build → Build Project
4. Build → Deploy

### Step 3: Install

```bash
adb install -r build-android/android-build/build/outputs/apk/debug/FastPayQt-debug.apk
```

---

## 📋 Prerequisites

1. **Qt 6.5+ with Android** (arm64-v8a, Qt Multimedia, OpenSSL for Android)
2. **Android SDK 34 + NDK 25+**
3. **Java JDK 17**

Install via Qt Online Installer: https://www.qt.io/download-qt-installer

---

## ⚙️ Configuration

The Android debug build defaults to the Android Emulator host address. Release builds have no server URL; enter a local demo server URL in the app when testing. Do not deploy the demo API publicly.

---

## 📱 Features

### Online Mode
- Ultrasound proximity payments
- Real-time verification
- Account freezing on mismatch

### Offline Mode
- Cold wallet transfers
- Digital signatures (ECDSA/RSA)
- Sync when back online

### Security
- UPI-style PIN (SHA-256 hashed)
- AES-256 encryption
- Transaction ID verification

---

## 🛠️ Troubleshooting

**OpenSSL not found?**
Install via Qt Maintenance Tool → Add Components → OpenSSL for Android

**Gradle errors?**
```bash
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64
```

**Permissions denied?**
App requests RECORD_AUDIO at runtime automatically

---

## 📦 Files to Edit Before Building

1. `server_config.h` - Leave the release server URL empty; enter a local demo API URL in the app when testing.
2. `android/AndroidManifest.xml` - Change package name if needed.

---

## Permissions

- RECORD_AUDIO - Ultrasound reception
- INTERNET - Online transactions
- MODIFY_AUDIO_SETTINGS - Audio control

---

Built with Qt 6 for demonstration and development.
