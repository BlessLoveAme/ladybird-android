# Chainload Android NDK under vcpkg for native C/C++ cross-compilation.
# The Gradle CMake invocation provides VCPKG_TARGET_ANDROID, ANDROID_NDK, ANDROID_ABI and VCPKG_ROOT.
if (NOT VCPKG_TARGET_ANDROID)
    message(FATAL_ERROR "Pass -DVCPKG_TARGET_ANDROID=ON for Android builds")
endif()
if (NOT DEFINED ANDROID_NDK OR NOT EXISTS "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")
    message(FATAL_ERROR "ANDROID_NDK must point to a complete Android NDK installation")
endif()
if (NOT DEFINED VCPKG_ROOT OR NOT EXISTS "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(FATAL_ERROR "VCPKG_ROOT must point to the initialized vcpkg checkout")
endif()
if (ANDROID_ABI STREQUAL "arm64-v8a")
    set(VCPKG_TARGET_TRIPLET "arm64-android" CACHE STRING "" FORCE)
elseif (ANDROID_ABI STREQUAL "x86_64")
    set(VCPKG_TARGET_TRIPLET "x64-android" CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "Unsupported Android ABI: ${ANDROID_ABI}")
endif()
set(VCPKG_HOST_TRIPLET "x64-windows" CACHE STRING "" FORCE)
# Keep Ladybird's distribution overlay ports and add the two Android target triplets.
set(VCPKG_OVERLAY_TRIPLETS
    "${CMAKE_CURRENT_LIST_DIR}/../../Meta/CMake/vcpkg/android-triplets;${CMAKE_CURRENT_LIST_DIR}/../../Meta/CMake/vcpkg/distribution-triplets"
    CACHE STRING "" FORCE)
# vcpkg's Android compiler-detection and target builds need the NDK toolchain explicitly.
set(ENV{ANDROID_NDK_HOME} "${ANDROID_NDK}")
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake" CACHE STRING "" FORCE)
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "" FORCE)
