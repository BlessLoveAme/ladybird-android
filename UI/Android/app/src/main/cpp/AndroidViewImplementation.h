// SPDX-License-Identifier: BSD-2-Clause
// Android ViewImplementation — SurfaceView backend for Ladybird WebContent rendering
#pragma once

#include <AK/Types.h>
#include <android/native_window.h>
#include <jni.h>

namespace Ladybird::Android {

class WebContentProcess;

// Android-specific WebView implementation backed by ANativeWindow (from SurfaceView)
// Handles compositor frame submission, input events, and IPC with WebContentProcess
class AndroidViewImplementation {
public:
    // Create WebView backed by Android Surface
    static AndroidViewImplementation* create(ANativeWindow* window);
    
    ~AndroidViewImplementation();

    // Navigate to URL (sends IPC message to WebContentProcess)
    void load_url(char const* url);

    // Input event dispatch (from Java touch/key events)
    void handle_touch_event(float x, float y, bool pressed);
    void handle_key_event(int key_code, bool pressed);
    void handle_resize(int width, int height);

    // Compositor callbacks (called by WebContentProcess via IPC)
    void request_repaint();
    void did_finish_loading(char const* url);

    ANativeWindow* native_window() const { return m_native_window; }

private:
    AndroidViewImplementation(ANativeWindow* window);
    bool initialize();

    ANativeWindow* m_native_window { nullptr };
    WebContentProcess* m_web_content_process { nullptr };
    int m_width { 0 };
    int m_height { 0 };
    // TODO: Skia/ANGLE surface for frame rendering
};

} // namespace Ladybird::Android
