# Configure Ladybird root CMake for Android engine build
# Requires vcpkg manifest installed at B:\Build\android-vcpkg-api30

$ErrorActionPreference = "Stop"

$LADYBIRD_ROOT = "B:\"
$BUILD_DIR = "B:\Build\android-engine-arm64"
$VCPKG_ROOT = "B:\Build\vcpkg"
$VCPKG_INSTALLED = "B:\Build\android-vcpkg-api30\vcpkg_installed"
$NDK_ROOT = "C:\Users\iadmin\AppData\Roaming\CherryStudio\Toolchain\mise\installs\vfox-mise-plugins-vfox-android-sdk\23.0\ndk\28.0.13004108"
$CMAKE_BIN = "C:\Users\iadmin\AppData\Roaming\CherryStudio\Toolchain\mise\installs\aqua-kitware-c-make\3.31.5\cmake-3.31.5-windows-x86_64\bin\cmake.exe"

Write-Host "==> Configuring Ladybird Android engine (arm64-v8a, API 30)" -ForegroundColor Cyan

# Check vcpkg manifest installed
if (-not (Test-Path "$VCPKG_INSTALLED\arm64-android")) {
    Write-Error "vcpkg manifest not installed at $VCPKG_INSTALLED/arm64-android — run vcpkg install first"
}

# Create build directory
New-Item -ItemType Directory -Force -Path $BUILD_DIR | Out-Null

# Configure root CMakeLists.txt with Android NDK + vcpkg
& $CMAKE_BIN `
    -S $LADYBIRD_ROOT `
    -B $BUILD_DIR `
    -G Ninja `
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
    -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE="$NDK_ROOT/build/cmake/android.toolchain.cmake" `
    -DVCPKG_OVERLAY_TRIPLETS="$LADYBIRD_ROOT/Meta/CMake/vcpkg/android-triplets" `
    -DVCPKG_TARGET_TRIPLET=arm64-android `
    -DVCPKG_INSTALLED_DIR="$VCPKG_INSTALLED" `
    -DVCPKG_MANIFEST_MODE=OFF `
    -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-30 `
    -DANDROID_STL=c++_shared `
    -DCMAKE_BUILD_TYPE=Release `
    -DENABLE_GUI_TARGETS=OFF `
    -DBUILD_LAGOM=OFF `
    -DVCPKG_TARGET_ANDROID=ON

Write-Host "==> CMake configure finished. Next: cmake --build $BUILD_DIR --target AK LibCore LibWeb LibJS" -ForegroundColor Green
