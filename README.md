# FastPay - Secure Payment Application

A Qt-based mobile payment application supporting both online and offline transactions with ultrasound-based proximity payments.

## 🚀 Quick Start - Build APK in 3 Steps

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

Built with Qt 6 • Secure by design 
