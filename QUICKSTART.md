# 🚀 FastPay - Quick Start Guide

## Build Your APK in 2 Minutes!

### Method 1: Automated (Easiest)

```bash
# Step 1: Configure
./configure.sh

# Step 2: Build
./build_android.sh

# Step 3: Done! Your APK is ready
```

### Method 2: Manual Configuration

1. **Edit server URL:**
   ```bash
   nano server_config.h
   ```
   Change this line:
   ```cpp
   const QString PRODUCTION_SERVER = "https://your-server-url.com";
   ```

2. **Build:**
   ```bash
   ./build_android.sh
   ```

3. **Install:**
   ```bash
   adb install -r build-android-arm64-v8a/android-build/build/outputs/apk/debug/FastPayQt-debug.apk
   ```

---

## 📋 Prerequisites Checklist

Before building, make sure you have:

- [ ] Qt 6.5+ installed with Android support
- [ ] Android SDK (API 34) and NDK (25+)
- [ ] Java JDK 17
- [ ] Your backend server URL ready

**Don't have these?** Download:
- Qt: https://www.qt.io/download-qt-installer
- Android Studio: https://developer.android.com/studio
- Java 17: `sudo apt install openjdk-17-jdk` (Ubuntu)

---

## ⚙️ What to Configure

### Required:
✅ **Server URL** in `server_config.h` - Your backend server address

### Optional:
- Package name in `android/AndroidManifest.xml` - Change if publishing to Play Store
- App name in `android/res/values/strings.xml` - Customize app display name
- Build type in `build_android.sh` - Debug or Release

---

## 🔧 Common Issues

**Issue: Qt not found**
```bash
# Edit build_android.sh and update:
QT_ANDROID_PATH="/path/to/your/Qt/6.x.x/android_arm64_v8a"
```

**Issue: NDK not found**
```bash
# List available NDK versions:
ls ~/Android/Sdk/ndk/

# Update build_android.sh with your version:
ANDROID_NDK_VERSION="your-version-here"
```

**Issue: Build failed - OpenSSL**
- Open Qt Maintenance Tool
- Add Components → OpenSSL for Android
- Rebuild

**Issue: Permission denied**
```bash
chmod +x configure.sh build_android.sh
```

---

## 📱 Testing

### On Emulator:
```bash
# Create emulator (one time)
avdmanager create avd -n Pixel6 -k "system-images;android-34;google_apis;arm64-v8a"

# Start emulator
emulator -avd Pixel6

# Install APK
adb install -r path/to/your.apk
```

### On Physical Device:
1. Enable Developer Options on your Android device
2. Enable USB Debugging
3. Connect via USB
4. Run `adb install -r your.apk`

---

## 🎯 Build Modes

### Debug Build (Default)
- Includes debug symbols
- Larger APK size
- Connects to `LOCAL_SERVER` (localhost)
- Use for testing

### Release Build
```bash
# Edit build_android.sh:
BUILD_TYPE="Release"

# Then build:
./build_android.sh
```
- Optimized & smaller
- Connects to `PRODUCTION_SERVER`
- Use for distribution

---

## 📦 What Gets Built

After successful build, you'll find:

```
build-android-arm64-v8a/
└── android-build/
    └── build/
        └── outputs/
            └── apk/
                └── debug/
                    └── FastPayQt-arm64-v8a-debug.apk  ← Your APK!
```

---

## 🆘 Need Help?

1. **Check logs:** `adb logcat | grep FastPay`
2. **Verify server:** `curl https://your-server-url.com/health`
3. **Clean build:** `rm -rf build-android-arm64-v8a && ./build_android.sh`

---

## ✅ Success Checklist

After building, verify:

- [ ] APK file exists and is > 10 MB
- [ ] Installs without errors
- [ ] App launches successfully
- [ ] Can set PIN
- [ ] Server connection works (check logs)

---

**Ready? Let's build! 🚀**

```bash
./configure.sh && ./build_android.sh
```
