# Using Qt for Android with FastPay

This project is a **Qt for Android** application. Build and run it using **Qt 6 for Android** (the Android target of the Qt framework).

---

## 1. Install Qt for Android

1. **Download the Qt Online Installer**  
   https://www.qt.io/download-qt-installer

2. **Run the installer** and select:
   - **Qt 6.x** (e.g. 6.6 or 6.7)
   - Under **Qt 6.x**, enable:
     - **Android** (or **Qt for Android**)
     - At least one Android ABI, e.g.:
       - **Android arm64-v8a** (for most phones/tablets)
       - **Android x86_64** (for emulator)

3. **Android SDK and NDK**  
   The installer can install them, or use existing ones (e.g. from Android Studio).  
   Typical locations:
   - Linux: `~/Android/Sdk`
   - NDK: `~/Android/Sdk/ndk/<version>`

4. **Java**  
   Qt for Android uses Gradle; install **JDK 17** (or the version recommended by your Qt).

---

## 2. Configure Android in Qt Creator

1. Open **Qt Creator**.
2. **Edit → Preferences** (or **Tools → Options** on Windows).
3. Go to **Devices → Android**.
4. Set **Android SDK path** and **JDK path** if not auto-detected.
5. Apply; Qt Creator will create an **Android** kit that uses Qt for Android.

---

## 3. Build and run with Qt for Android

1. **File → Open File or Project**.
2. Open **`CMakeLists.txt`** (recommended) or **`FastPayQt.pro`** in the `FastPayQt` folder.
3. When asked for a **kit**, choose the **Android** kit, e.g.:
   - **Android Qt 6.6.0 arm64-v8a**
   - or **Android Qt 6.6.0 x86_64** for the emulator.
4. **Build → Build Project** (Ctrl+B).
5. **Run → Run** (Ctrl+R) to run on a connected device or emulator.

The first run may take a while (Gradle, deployment). The APK is produced under the build directory (e.g. `build-android/android-build/.../apk/`).

---

## 4. Command line (Qt for Android)

Use the **qt-cmake** from the **Qt for Android** installation (not the desktop Qt):

```bash
# Set paths (adjust versions to yours)
export ANDROID_SDK_ROOT=~/Android/Sdk
export ANDROID_NDK_ROOT=~/Android/Sdk/ndk/25.1.8937393

# Configure with Qt for Android (arm64-v8a)
cd /path/to/Transaction/FastPayQt
mkdir -p build-android && cd build-android

~/Qt/6.6.0/android_arm64_v8a/bin/qt-cmake -S .. -B .

# Build APK
cmake --build . --target apk
```

Then install and run:

```bash
adb install -r android-build/FastPayQt/build/outputs/apk/debug/FastPayQt-debug.apk
adb shell am start -n com.fastpay.transaction/org.qtproject.qt.android.bindings.QtActivity
```

---

## 5. What “Qt for Android” means here

- The app is built with **Qt 6** (Core, Gui, Widgets, Network).
- The **Android** build uses the **Qt for Android** toolchain and **androiddeployqt** to produce an APK.
- The `android/` folder (manifest, Gradle, resources) is used as the Android package source (`QT_ANDROID_PACKAGE_SOURCE_DIR`).
- No native Android (Java/Kotlin) UI; the UI is **Qt Widgets** and runs on Android via Qt for Android.

For more details (troubleshooting, permissions, multi-ABI), see **BUILD_ANDROID.md**.
