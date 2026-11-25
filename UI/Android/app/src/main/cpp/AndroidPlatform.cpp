// SPDX-License-Identifier: BSD-2-Clause
#include "AndroidPlatform.h"
#include <android/log.h>

#define LOG_TAG "AndroidPlatform"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// TODO: When LibWeb is linked, uncomment:
// #include <LibWebView/Application.h>
// #include <LibCore/EventLoop.h>
// #include <LibWeb/Bindings/MainThreadVM.h>

namespace Ladybird::Android {

static AndroidApplication* s_application = nullptr;

AndroidApplication& AndroidApplication::the()
{
    if (!s_application) {
        s_application = new AndroidApplication();
    }
    return *s_application;
}

bool AndroidApplication::initialize()
{
    LOGI("AndroidApplication::initialize");
    
    // TODO: When LibWeb is linked:
    // 1. Set up resource loader callbacks for Android assets
    // 2. Configure font paths to Android system fonts (/system/fonts/)
    // 3. Initialize LibCore::EventLoop for main thread
    // 4. Set up Web::Bindings::ensure_web_namespace_initialized()
    
    [[maybe_unused]] auto& app = the();
    return true;
}

void AndroidApplication::shutdown()
{
    LOGI("AndroidApplication::shutdown");
    
    delete s_application;
    s_application = nullptr;
    
    // TODO: Cleanup LibCore::EventLoop, LibWeb global state
}

AndroidApplication::AndroidApplication()
{
    LOGI("AndroidApplication created");
}

AndroidApplication::~AndroidApplication()
{
    LOGI("AndroidApplication destroyed");
}

void AndroidPlatformAdapter::initialize_for_view()
{
    LOGI("AndroidPlatformAdapter::initialize_for_view");
    
    // TODO: Set up LibWeb platform hooks:
    // - Font loading via Android system fonts
    // - Image decoding via Skia or BitmapFactory
    // - Timer scheduling via LibCore::EventLoop
}

void* AndroidPlatformAdapter::load_system_font([[maybe_unused]] char const* family_name, 
    [[maybe_unused]] int weight, [[maybe_unused]] bool italic)
{
    // TODO: Load font from /system/fonts/ using FreeType or Android FontFamily API
    // Return FT_Face or Skia SkTypeface
    return nullptr;
}

void* AndroidPlatformAdapter::decode_image([[maybe_unused]] void const* data, [[maybe_unused]] size_t size)
{
    // TODO: Decode PNG/JPEG/GIF/WebP using Skia or stb_image
    // Return SkBitmap or LibGfx::Bitmap
    return nullptr;
}

void AndroidPlatformAdapter::schedule_timer([[maybe_unused]] int milliseconds, 
    [[maybe_unused]] void (*callback)(void*), [[maybe_unused]] void* userdata)
{
    // TODO: Use LibCore::Timer or Android Handler.postDelayed
}

} // namespace Ladybird::Android
