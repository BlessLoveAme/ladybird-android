// SPDX-License-Identifier: BSD-2-Clause
// Android Platform Adapter for LibWebView — provides Android-specific implementations
#pragma once

#include <android/native_window.h>

// Forward declarations (will be linked when LADYBIRD_LINK_ENGINE=ON)
// #include <LibWebView/Application.h>
// #include <LibWebView/ViewImplementation.h>
// #include <LibWeb/Page/Page.h>

namespace Ladybird::Android {

// Android-specific WebView Application singleton
// Manages global LibWeb state, resource loading, font configuration
class AndroidApplication /* : public WebView::Application */ {
public:
    static AndroidApplication& the();
    
    // Initialize application (called once at app startup)
    static bool initialize();
    static void shutdown();

    // Resource loading callbacks for LibWeb
    // Load file:// URLs from Android assets
    // Load http(s):// URLs via Android URLConnection or native sockets
    
private:
    AndroidApplication();
    ~AndroidApplication();
};

// Android-specific ViewImplementation platform hooks
// Provides font rendering, image decoding, timers, event loop integration
class AndroidPlatformAdapter {
public:
    static void initialize_for_view();
    
    // Font loading from Android system fonts
    static void* load_system_font(char const* family_name, int weight, bool italic);
    
    // Image decoding using Android BitmapFactory or Skia
    static void* decode_image(void const* data, size_t size);
    
    // Timers using LibCore::EventLoop integrated with Android Looper
    static void schedule_timer(int milliseconds, void (*callback)(void*), void* userdata);
};

} // namespace Ladybird::Android
