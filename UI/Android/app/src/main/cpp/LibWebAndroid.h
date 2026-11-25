// SPDX-License-Identifier: BSD-2-Clause
// LibWeb Page initialization for Android — creates JS VM, HTML parser, layout engine
#pragma once

// Forward declarations (will be linked when LADYBIRD_LINK_ENGINE=ON)
// #include <LibWeb/Page/Page.h>
// #include <LibJS/Runtime/VM.h>
// #include <LibCore/EventLoop.h>

namespace Ladybird::Android {

// Initialize LibWeb for Android WebContent thread
// Returns true if LibWeb/LibJS successfully initialized
bool initialize_libweb_for_android();

// Create LibWeb::Page for rendering
// Returns opaque handle (LibWeb::Page*) or nullptr on failure
void* create_web_page(int viewport_width, int viewport_height);

// Navigate page to URL
void web_page_load_url(void* page_handle, char const* url);

// Handle viewport resize
void web_page_resize(void* page_handle, int width, int height);

// Dispatch mouse event to page
void web_page_mouse_event(void* page_handle, int x, int y, int button, bool pressed);

// Render page to Skia canvas (returns frame buffer for compositor)
void* web_page_render(void* page_handle);

// Destroy LibWeb::Page
void destroy_web_page(void* page_handle);

// Shutdown LibWeb runtime
void shutdown_libweb();

} // namespace Ladybird::Android
