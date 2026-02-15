# Building FastPay for Android

FastPay can be built as an Android APK using **Qt 6 for Android** and either **Qt Creator** or the **command line (CMake)**.

---

## Prerequisites

1. **Qt 6** with **Android** support  
   - Install via [Qt Online Installer](https://www.qt.io/download-qt-installer) and select:
     - Qt 6.x
     - **Qt for Android** (e.g. Android arm64-v8a, Android x86_64 for emulator)

2. **Android SDK and NDK**  
   - Usually installed automatically with “Qt for Android”  
   - Or install [Android Studio](https://developer.android.com/studio) and use its SDK/NDK  
   - Typical paths:
     - Linux: `~/Android/Sdk/`
     - SDK path is often: `~/Android/Sdk`
     - NDK path is often: `~/Android/Sdk/ndk/<version>`

3. **Java JDK** (e.g. OpenJDK 17) for Gradle

---

## Option A: Build with Qt Creator (recommended)

1. Open **Qt Creator**.
2. **File → Open File or Project** and choose:
   - `FastPayQt/CMakeLists.txt`  
   or  
   - `FastPayQt/FastPayQt.pro`
3. Select the **Android** kit (e.g. “Android Qt 6.x.x arm64-v8a”) and configure the project.
4. Build: **Build → Build Project** (e.g. Ctrl+B).
5. Run on device/emulator: **Run → Run** (e.g. Ctrl+R), or **Build → Deploy** to get the APK.

The generated APK is typically under the build directory in something like  
`android-build/build/outputs/apk/...` (exact path depends on Qt version).

---

## Option B: Build from command line with CMake

Use the **Qt for Android** `qt-cmake` and `cmake` for your target ABI (e.g. `arm64-v8a`).

1. Set paths (adjust to your Qt and SDK/NDK):

   ```bash
   export Qt6_DIR=~/Qt/6.6.0/android_arm64_v8a/lib/cmake/Qt6
   export ANDROID_SDK_ROOT=~/Android/Sdk
   export ANDROID_NDK_ROOT=~/Android/Sdk/ndk/25.1.8937393
   ```

   Or use the `qt-cmake` that comes with Qt for Android:

   ```bash
   export PATH=~/Qt/6.6.0/android_arm64_v8a/bin:$PATH
   ```

2. Configure for Android (from the **FastPayQt** directory):

   ```bash
   cd /path/to/Transaction/FastPayQt
   mkdir -p build-android && cd build-android

   ~/Qt/6.6.0/android_arm64_v8a/bin/qt-cmake \
     -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
     -DANDROID_ABI=arm64-v8a \
     -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
     -DANDROID_NDK_ROOT=$ANDROID_NDK_ROOT \
     -S .. -B .
   ```

   If your Qt install provides a wrapper that sets the toolchain for you, you may only need:

   ```bash
   ~/Qt/6.6.0/android_arm64_v8a/bin/qt-cmake -S .. -B .
   ```

3. Build the APK:

   ```bash
   cmake --build . --target apk
   ```

4. Install and run on a connected device:

   ```bash
   adb install -r android-build/FastPayQt/build/outputs/apk/debug/FastPayQt-debug.apk
   adb shell am start -n com.fastpay.transaction/org.qtproject.qt.android.bindings.QtActivity
   ```

Paths (e.g. `android-build`, APK name) can vary with Qt version; check the build output.

---

## Option C: Build with qmake (single ABI)

If you use **qmake** with an Android kit in Qt Creator:

1. Open `FastPayQt.pro` in Qt Creator.
2. Select an **Android** kit and build.
3. Deploy to device or locate the APK from the build directory.

Note: With qmake, multi-ABI builds are not supported; use CMake for multiple ABIs.

---

## Android-specific files in this project

| Path | Purpose |
|------|--------|
| `android/AndroidManifest.xml` | Package name, permissions (RECORD_AUDIO, INTERNET), app name |
| `android/build.gradle` | Gradle build config (minSdk, targetSdk, applicationId) |
| `android/gradle.properties` | Gradle options |
| `android/res/values/strings.xml` | App name string |
| `CMakeLists.txt` | Sets `QT_ANDROID_PACKAGE_SOURCE_DIR` to `android/` |

---

## Permissions

- **RECORD_AUDIO** and **MODIFY_AUDIO_SETTINGS**: for ultrasound send/receive.
- **INTERNET**: for online transactions and sync.

On Android 6+ (API 23+), RECORD_AUDIO should be requested at runtime before using the microphone; the manifest only declares the permission. You can add a runtime request (e.g. when the user taps “Start listening”) using Qt’s JNI (e.g. `QJniObject`) or Qt 6.5+ permission APIs.

---

## Troubleshooting

- **“SDK/NDK not found”**: Set `ANDROID_SDK_ROOT` and `ANDROID_NDK_ROOT`, or configure them in Qt Creator (Options → Devices → Android).
- **“No Android kit”**: In Qt Creator, add an Android kit that uses your Qt for Android and the same SDK/NDK.
- **Gradle errors**: Ensure Java (e.g. JDK 17) is installed and `JAVA_HOME` is set.
- **APK not found**: After `cmake --build . --target apk`, look under `android-build/` for `*.apk` (exact path may vary; check the build log).
