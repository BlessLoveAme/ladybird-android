cmake_minimum_required(VERSION 3.25)
project(LadybirdAndroid LANGUAGES CXX)
if (NOT ANDROID)
    message(FATAL_ERROR "Use the Android NDK toolchain")
endif()

# Ladybird root checkout and vcpkg toolchain
get_filename_component(LADYBIRD_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../../../../" ABSOLUTE)
if (NOT EXISTS "${LADYBIRD_ROOT}/AK/Platform.h")
    message(FATAL_ERROR "Build UI/Android inside the Ladybird checkout")
endif()

# Option: link full engine or build bootstrap smoke APK
option(LADYBIRD_LINK_ENGINE "Link LibWeb/LibJS/WebContent (requires vcpkg dependencies)" OFF)

if(LADYBIRD_LINK_ENGINE)
    # Full engine build — requires vcpkg manifest installed at root
    if(NOT VCPKG_MANIFEST_MODE)
        message(FATAL_ERROR "LADYBIRD_LINK_ENGINE requires vcpkg manifest mode")
    endif()
    
    # Include root CMakeLists to bring AK/LibCore/LibWeb targets
    # Note: Ladybird root CMake expects ENABLE_GUI_TARGETS=ON for LibWebView
    add_subdirectory("${LADYBIRD_ROOT}" "${CMAKE_CURRENT_BINARY_DIR}/ladybird" EXCLUDE_FROM_ALL)
    
    add_library(ladybird_android SHARED
        LadybirdEngine.cpp
        AndroidViewImplementation.cpp
        WebContentProcess.cpp
        LibWebAndroid.cpp
        AndroidPlatform.cpp
    )
    target_compile_features(ladybird_android PRIVATE cxx_std_23)
    target_compile_options(ladybird_android PRIVATE -Wall -Wextra -Werror)
    
    # Link Ladybird engine libs
    target_link_libraries(ladybird_android PRIVATE
        AK
        LibCore
        LibIPC
        LibWeb
        LibJS
        LibWebView
    )
    
    # Android platform libs
    target_link_libraries(ladybird_android PRIVATE log android)
else()
    # Bootstrap smoke APK — minimal AK test without engine
    add_library(ladybird_android_bootstrap SHARED
        NativeBridge.cpp
        WebContentProcess.cpp
        AndroidViewImplementation.cpp
        LibWebAndroid.cpp
        AndroidPlatform.cpp
        "${LADYBIRD_ROOT}/AK/StackInfo.cpp"
    )
    target_compile_features(ladybird_android_bootstrap PRIVATE cxx_std_23)
    target_include_directories(ladybird_android_bootstrap PRIVATE "${LADYBIRD_ROOT}")
    target_compile_options(ladybird_android_bootstrap PRIVATE -Wall -Wextra -Werror)
    target_link_libraries(ladybird_android_bootstrap PRIVATE log android)
endif()
