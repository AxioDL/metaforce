# Android

## Prerequisites

- Android SDK with Platform 37 installed (`ANDROID_HOME`)
- Android NDK (`ANDROID_NDK_VERSION`)
- JDK 17+

```bash
export ANDROID_HOME="$HOME/Android/Sdk"
export ANDROID_NDK_VERSION="30.0.16248370"
```

## Build

Build the native library first:

```bash
cmake --preset android-arm64
cmake --build --preset android-arm64
```

Then build the APK:

```bash
cd platforms/android
./gradlew :app:assembleDebug
```

Output: `app/build/outputs/apk/debug/app-arm64-v8a-debug.apk`

## Running

The data directory is exposed in the Files app under "Metaforce".

Pass command-line arguments through the launch intent:

```bash
adb shell am start -n com.axiodl.metaforce/.MetaforceActivity \
  --es borealis_args "--warp 2,2"
```

Aurora needs a hardware-backed graphics adapter. Start AVDs with `-gpu host`.
