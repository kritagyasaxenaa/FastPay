# FastPay - Payment Flow Prototype

A Qt-based C++ prototype exploring online and offline payment flows with ultrasound-based proximity discovery. It is not a real payment product and must not be used to move money or store production credentials.

## Production status

**Not production-ready.** The Android app contains demo transaction flows and placeholder cryptography. The server does not authenticate users, enforce balances, or verify offline signatures. The production-named API now disables all `/api/v1/` operations by default; only `/` and `/api/v1/health` remain available. Setting `FASTPAY_DEMO_MODE=true` re-enables the prototype API for isolated demonstrations only. Never set that flag on a public deployment.

The current Android app uses Qt Widgets. It is a C++ app that can be packaged for Android, but it does not use Android's native UI toolkit. A native Android UI would require a Kotlin/Java layer and a deliberate JNI boundary to the C++ logic.

## Simulated bank demo

For local demonstrations, run the API with `FASTPAY_DEMO_MODE=true`. It seeds test accounts with INR 1,000.00 and simulates transfers by debiting and crediting the in-app ledger. The transfer API supports retries without duplicate debits and exposes demo balances at `/api/v1/demo/bank-accounts/{account_id}`. It is play money only; it does not connect to banks, UPI, or other payment rails. Keep demo mode off on public deployments.

## Build the Android APK

### Step 1: Configure Server URL

Edit `server_config.h` and change the PRODUCTION_SERVER URL:

```cpp
const QString PRODUCTION_SERVER = "https://your-actual-server-url.com";
```

### Step 2: Build APK

Run the automated build script:
```bash
chmod +x build_android.sh
./build_android.sh
```

Or use Qt Creator:
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

**IMPORTANT:** Edit `server_config.h` before building:

```cpp
const QString PRODUCTION_SERVER = "https://your-server-url.com";
```

Deploy the Python server (in `server/` directory) to Render, Railway, or your own hosting.

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

1. ✅ `server_config.h` - Set your production server URL
2. ✅ `android/AndroidManifest.xml` - Change package name if needed
3.  `build_android.sh` - Update Qt/SDK paths if non-standard

---

## Permissions

- RECORD_AUDIO - Ultrasound reception
- INTERNET - Online transactions
- MODIFY_AUDIO_SETTINGS - Audio control

---

Built with Qt 6 for demonstration and development.
