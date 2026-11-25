// SPDX-License-Identifier: BSD-2-Clause
#include "LibWebAndroid.h"
#include "AndroidPlatform.h"
#include <android/log.h>

#define LOG_TAG "LibWebAndroid"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// TODO: When LADYBIRD_LINK_ENGINE=ON, uncomment these includes:
// #include <LibWeb/Page/Page.h>
// #include <LibJS/Runtime/VM.h>
// #include <LibCore/EventLoop.h>
// #include <LibWeb/HTML/HTMLDocument.h>
// #include <LibWeb/DOM/Document.h>
// #include <LibWeb/Bindings/MainThreadVM.h>
// #include <LibURL/URL.h>

namespace Ladybird::Android {

bool initialize_libweb_for_android()
{
    LOGI("initialize_libweb_for_android called");
    
    // Initialize Android platform adapter
    if (!AndroidApplication::initialize()) {
        LOGE("Failed to initialize AndroidApplication");
        return false;
    }
    
    AndroidPlatformAdapter::initialize_for_view();
    
    // TODO: When LibWeb is linked:
    // 1. Initialize LibCore::EventLoop for this thread
    // 2. Call LibWeb::Bindings::ensure_web_namespace_initialized()
    // 3. Create JS::VM with Android platform hooks
    // 4. Register Web IDL bindings
    
    // For now, just return false until LibWeb is linked
    LOGE("LibWeb not linked — returning false");
    return false;
}

void* create_web_page(int viewport_width, int viewport_height)
{
    LOGI("create_web_page: %dx%d", viewport_width, viewport_height);
    
    // TODO: When LibWeb is linked:
    // auto page = make<LibWeb::Page>(/* Android platform adapter */);
    // page->set_viewport_size({ viewport_width, viewport_height });
    // return page.leak_ptr();
    
    return nullptr;
}

void web_page_load_url(void* page_handle, char const* url)
{
    LOGI("web_page_load_url: %s", url);
    
    if (!page_handle) {
        LOGE("web_page_load_url: null page handle");
        return;
    }
    
    // TODO: When LibWeb is linked:
    // auto* page = static_cast<LibWeb::Page*>(page_handle);
    // auto url_or_error = URL::URL::create_with_url_or_path(url);
    // if (url_or_error.is_error()) {
    //     LOGE("Invalid URL: %s", url);
    //     return;
    // }
    // page->load(url_or_error.release_value());
}

void web_page_resize(void* page_handle, int width, int height)
{
    LOGI("web_page_resize: %dx%d", width, height);
    
    if (!page_handle) return;
    
    // TODO: When LibWeb is linked:
    // auto* page = static_cast<LibWeb::Page*>(page_handle);
    // page->set_viewport_size({ width, height });
}

void web_page_mouse_event(void* page_handle, [[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int button, [[maybe_unused]] bool pressed)
{
    if (!page_handle) return;
    
    // TODO: When LibWeb is linked:
    // auto* page = static_cast<LibWeb::Page*>(page_handle);
    // Create LibWeb::UIEvents::MouseEvent and dispatch to document
}

void* web_page_render(void* page_handle)
{
    if (!page_handle) return nullptr;
    
    // TODO: When LibWeb is linked + Skia compositor ready:
    // auto* page = static_cast<LibWeb::Page*>(page_handle);
    // 1. Run layout if needed: page->document()->update_layout()
    // 2. Create Skia canvas backed by ANativeWindow buffer
    // 3. Paint page to canvas: page->paint_tree()->paint_all_phases()
    // 4. Return frame buffer for ANativeWindow_unlockAndPost
    
    return nullptr;
}

void destroy_web_page(void* page_handle)
{
    LOGI("destroy_web_page");
    
    if (!page_handle) return;
    
    // TODO: When LibWeb is linked:
    // auto* page = static_cast<LibWeb::Page*>(page_handle);
    // delete page;
}

void shutdown_libweb()
{
    LOGI("shutdown_libweb");
    
    AndroidApplication::shutdown();
    
    // TODO: When LibWeb is linked:
    // Cleanup JS::VM, LibCore::EventLoop, global state
}

} // namespace Ladybird::Android
