# Ladybird Android Port

Experimental Android port of the [Ladybird browser](https://github.com/LadybirdBrowser/ladybird) engine — LibWeb/LibJS running on Android with Material 3 UI.

> **Status:** Browser architecture complete. vcpkg dependency manifest building (ANGLE, ffmpeg, ICU). LibWeb/LibJS will be linked once manifest finishes.

## What's here

This repository contains the Android-specific port files only. It is meant to be applied on top of the upstream Ladybird source tree (commit `60393bb3`).

### `UI/Android/` — Android app
- Material 3 Compose browser UI (address bar, SurfaceView, navigation)
- JNI bridge: `NativeBridge.kt` ↔ `LadybirdEngine.cpp`
- `AndroidViewImplementation` — ANativeWindow management
- `WebContentProcess` — dedicated pthread with Unix pipe IPC
- `LibWebAndroid` — integration layer for LibWeb::Page / JS::VM
- `AndroidPlatform` — font/image/timer adapter
- `WebContentService` — Android Service for renderer isolation

### `patches/` — Git patches
14 patches to apply on upstream Ladybird `60393bb3`:
- NDK bringup, APK build
- LibCore Android notifier fix
- CMake linker fix for Android NDK
- vcpkg Android triplets (arm64, x64, API 30)
- Full engine integration (JNI, ViewImplementation, WebContent IPC)

### `Meta/CMake/` — Patched CMake files
- `use_linker.cmake` — Android NDK linker fallback
- `check_for_dependencies.cmake` — skip PkgConfig on Android
- `environment.cmake` — overlay triplets for Android
- `vcpkg/android-triplets/` — arm64-android and x64-android static triplets

### `Libraries/LibCore/` — Patched upstream file
- `EventLoopImplementationUnix.cpp` — Android notifier readiness fix

## Applying to upstream Ladybird

```bash
# Clone upstream at base commit
git clone https://github.com/LadybirdBrowser/ladybird.git
cd ladybird
git checkout 60393bb3

# Apply all patches
git am patches/0001-*.patch
git am patches/0002-*.patch
# ... through 0014

# Or apply all at once
git am patches/*.patch
```

## Building

### Requirements
- JDK 17 (Temurin), Android SDK 23.0, NDK 28.0.13004108, CMake 3.31.5
- Windows 10/11 + Visual Studio Build Tools 2022 (for vcpkg host tools)
- vcpkg at `Build/vcpkg` baseline `8f363bdc`

### Bootstrap APK (no LibWeb yet)
```powershell
cd UI/Android
$env:JAVA_HOME = '<jdk17 path>'
$env:ANDROID_HOME = '<sdk path>'
.\gradlew.bat assembleDebug
# Output: app/build/outputs/apk/debug/app-debug.apk (~25 MB)
```

### Full engine APK (after vcpkg manifest)
```powershell
# 1. Configure root CMake
.\configure-android-engine.ps1

# 2. Build Ladybird libs
cmake --build Build\android-engine-arm64 --target AK LibCore LibWeb LibJS

# 3. Enable engine in Gradle (set LADYBIRD_LINK_ENGINE=ON)
# Edit UI/Android/app/build.gradle.kts

# 4. Build APK
cd UI/Android && .\gradlew.bat assembleDebug
```

## Architecture

```
MainActivity (Kotlin/Compose)
  └─ LadybirdWebView  ← Compose AndroidView wrapper
      └─ SurfaceView
          └─ NativeBridge JNI
              └─ AndroidViewImplementation (C++)
                  ├─ ANativeWindow (compositor — clears to white now, Skia/ANGLE next)
                  └─ WebContentProcess (pthread)
                      ├─ Unix pipe IPC (LoadURL / Resize / MouseEvent / Paint)
                      ├─ LibWebAndroid → LibWeb::Page + JS::VM (stubs → real after vcpkg)
                      └─ AndroidPlatform → fonts, images, timers
  + WebContentService (Android :webcontent process, Binder IPC TODO)
```

## Browser milestone criteria (from PORTING.md)
1. LibWeb and LibJS actually link into Android helper libraries — **in progress (vcpkg)**
2. A local HTML test page renders through LibWebView and an Android surface — **next**
3. Touch, keyboard, back navigation and Activity pause/resume work — **UI stubs ready**
4. HTTPS navigation with certificate validation — **curl+openssl in vcpkg manifest**
5. Process failure reported without killing UI — **WebContentService isolates renderer**

## License
BSD-2-Clause — same as upstream Ladybird.
