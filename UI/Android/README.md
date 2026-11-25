# Ladybird Android Port

Experimental Android port of the Ladybird web browser engine (LibWeb/LibJS).

## Status

**Current**: WebContent rendering architecture complete, Material 3 UI ready, waiting for vcpkg manifest to link LibWeb/LibJS.

### Working
- ✅ Material 3 Compose browser UI (address bar, navigation, SurfaceView)
- ✅ JNI bridge (Java ↔ C++)
- ✅ WebContent process thread with IPC (Unix pipe)
- ✅ AndroidViewImplementation with ANativeWindow management
- ✅ LibWebAndroid API stubs (Page creation, URL loading, rendering)
- ✅ AndroidPlatform adapter (font/image loading, timers)
- ✅ Bootstrap APK builds and runs (~25MB)

### In Progress
- 🔄 vcpkg manifest install (ANGLE ✅, ffmpeg building, ~37 packages remaining)
- ⏳ LibWeb/LibJS linking (after vcpkg completes)

### Not Yet Implemented
- ⏳ HTML parsing and layout (LibWeb::Page)
- ⏳ JavaScript execution (JS::VM)
- ⏳ Skia/ANGLE compositor (render to ANativeWindow)
- ⏳ Font rendering (Android system fonts)
- ⏳ Image decoding (PNG/JPEG/WebP)
- ⏳ Network requests (URLConnection or sockets)

## Build Requirements

- **JDK 17** (Temurin recommended)
- **Android SDK 23.0** with NDK 28.0.13004108
- **CMake 3.31.5+**
- **Windows 10/11** with Visual Studio Build Tools 2022 (MSVC 14.44)
- **vcpkg** at `Build/vcpkg` (baseline `8f363bdc`)

## Build Instructions

### 1. Bootstrap APK (without LibWeb)

```powershell
cd UI/Android
$env:JAVA_HOME='<path-to-jdk-17>'
$env:ANDROID_HOME='<path-to-android-sdk>'
.\gradlew.bat assembleDebug
```

Output: `UI/Android/app/build/outputs/apk/debug/app-debug.apk`

This builds a minimal APK with:
- WebContent process thread (spawns successfully)
- IPC pipe (LoadURL/Resize messages work)
- SurfaceView (clears to white on navigation)
- **No HTML rendering** (LibWeb not linked)

### 2. Full Engine APK (after vcpkg manifest)

#### Step 1: Wait for vcpkg manifest to complete

vcpkg install is running in background:
```powershell
Get-Process vcpkg,cmake
# Wait until complete (output: "Elapsed time for package xxx")
```

#### Step 2: Configure Ladybird root CMake

```powershell
.\configure-android-engine.ps1
# Output: B:\Build\android-engine-arm64\
```

#### Step 3: Build Ladybird libraries

```powershell
cmake --build B:\Build\android-engine-arm64 --target AK LibCore LibIPC LibWeb LibJS LibWebView
```

#### Step 4: Enable engine in Gradle

Edit `UI/Android/app/build.gradle.kts`:
```kotlin
externalNativeBuild {
    cmake {
        arguments += "-DLADYBIRD_LINK_ENGINE=ON"
        arguments += "-DCMAKE_TOOLCHAIN_FILE=B:/Build/vcpkg/scripts/buildsystems/vcpkg.cmake"
        arguments += "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=$ndkRoot/build/cmake/android.toolchain.cmake"
        arguments += "-DVCPKG_TARGET_TRIPLET=arm64-android"
    }
}
```

#### Step 5: Build full engine APK

```powershell
cd UI/Android
.\gradlew.bat assembleDebug
```

Output: `app-debug.apk` with LibWeb/LibJS linked (~100-200MB).

#### Step 6: Uncomment LibWeb code

Edit `UI/Android/app/src/main/cpp/LibWebAndroid.cpp`:
- Uncomment `#include <LibWeb/Page/Page.h>` etc.
- Implement `initialize_libweb_for_android()`: LibCore::EventLoop, JS::VM, Web bindings
- Implement `create_web_page()`: `LibWeb::Page` with Android adapter
- Implement `web_page_load_url()`: URL parsing, HTML document loading
- Implement `web_page_render()`: Layout + Paint → Skia → ANativeWindow

Rebuild and test HTML rendering.

## Architecture

```
MainActivity (Kotlin)
  └─> LadybirdWebView (Compose AndroidView)
       └─> SurfaceView
            └─> NativeBridge JNI
                 └─> AndroidViewImplementation (C++)
                      ├─> ANativeWindow (compositor)
                      └─> WebContentProcess (pthread)
                           ├─> IPC pipe (LoadURL/Resize/Paint)
                           ├─> LibWebAndroid API
                           │    ├─> LibWeb::Page
                           │    ├─> JS::VM
                           │    └─> LibCore::EventLoop
                           └─> AndroidPlatform adapter
                                ├─> Font loading (/system/fonts/)
                                ├─> Image decoding (Skia)
                                └─> Timers (LibCore)
```

## Known Issues

1. **vcpkg manifest slow on Windows**: ffmpeg configure takes 10-15 minutes due to MSYS2 overhead. Use `subst B: <path>` to shorten paths.
2. **LibSandbox Linux-only**: Android port disables process sandboxing (uses threads instead of fork+exec).
3. **No IPC process isolation**: WebContent runs in pthread, not separate process (Android doesn't support traditional fork model).
4. **Font rendering**: Needs FreeType + Android system font discovery.
5. **Network requests**: Needs Android URLConnection integration or native sockets.

## Testing

### Bootstrap APK
1. Install APK on device/emulator: `adb install -r app-debug.apk`
2. Launch app
3. Tap INFO button → check "Native engine status"
4. Enter URL in address bar (e.g., `example.com`)
5. Observe white Surface (placeholder rendering)
6. Check logcat: `adb logcat | grep Ladybird`

Expected logs:
```
LadybirdAndroid: AndroidViewImplementation created
WebContent: WebContent thread started
WebContent: LoadURL IPC: https://example.com
LibWebAndroid: LibWeb not linked — returning false
LadybirdAndroid: request_repaint
```

### Full Engine APK (after LibWeb linked)
1. Same steps as bootstrap
2. INFO screen should show "Ladybird Engine: LINKED"
3. Navigate to `example.com` → HTML should render
4. Check logcat for LibWeb parsing/layout logs

## Contributing

This is an experimental port. Key areas for contribution:
- Skia/ANGLE compositor integration
- Font rendering (FreeType + Android fonts)
- JavaScript debugging (JS::VM Android integration)
- Network stack (URLConnection or sockets)
- Input handling (touch → WebContent mouse events)

## License

BSD-2-Clause (same as Ladybird upstream)
