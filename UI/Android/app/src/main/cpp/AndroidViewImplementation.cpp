// SPDX-License-Identifier: BSD-2-Clause
#include "AndroidViewImplementation.h"
#include "WebContentProcess.h"
#include <android/log.h>
#include <cstdlib>

#define LOG_TAG "LadybirdAndroid"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace Ladybird::Android {

AndroidViewImplementation* AndroidViewImplementation::create(ANativeWindow* window)
{
    if (!window) {
        LOGE("AndroidViewImplementation::create: null ANativeWindow");
        return nullptr;
    }
    auto* view = new AndroidViewImplementation(window);
    if (!view->initialize()) {
        delete view;
        return nullptr;
    }
    return view;
}

AndroidViewImplementation::AndroidViewImplementation(ANativeWindow* window)
    : m_native_window(window)
{
    ANativeWindow_acquire(m_native_window);
    m_width = ANativeWindow_getWidth(m_native_window);
    m_height = ANativeWindow_getHeight(m_native_window);
    LOGI("AndroidViewImplementation created with ANativeWindow %p (%dx%d)", window, m_width, m_height);
}

bool AndroidViewImplementation::initialize()
{
    // Spawn WebContent rendering process
    m_web_content_process = WebContentProcess::create(this);
    if (!m_web_content_process) {
        LOGE("Failed to create WebContent process");
        return false;
    }

    // Send initial viewport size
    m_web_content_process->handle_resize(m_width, m_height);

    // TODO: Initialize Skia/ANGLE GL context for compositor rendering
    return true;
}

AndroidViewImplementation::~AndroidViewImplementation()
{
    delete m_web_content_process;
    m_web_content_process = nullptr;

    if (m_native_window) {
        ANativeWindow_release(m_native_window);
        m_native_window = nullptr;
    }
    LOGI("AndroidViewImplementation destroyed");
}

void AndroidViewImplementation::load_url(char const* url)
{
    LOGI("load_url: %s", url);
    if (m_web_content_process) {
        m_web_content_process->load_url(url);
    }
}

void AndroidViewImplementation::handle_touch_event(float x, float y, bool pressed)
{
    // Convert Android touch to mouse event for LibWeb
    if (m_web_content_process) {
        m_web_content_process->handle_mouse_event(static_cast<int>(x), static_cast<int>(y), 0, pressed);
    }
}

void AndroidViewImplementation::handle_key_event([[maybe_unused]] int key_code, [[maybe_unused]] bool pressed)
{
    // TODO: Convert Android KeyEvent to WebContent KeyEvent and send via IPC
}

void AndroidViewImplementation::handle_resize(int width, int height)
{
    m_width = width;
    m_height = height;
    if (m_web_content_process) {
        m_web_content_process->handle_resize(width, height);
    }
}

void AndroidViewImplementation::request_repaint()
{
    // TODO: Called by WebContent IPC when page needs redraw
    // Lock ANativeWindow buffer, submit frame from WebContent compositor
    LOGI("request_repaint");

    // For now, just clear to white to show the surface is active
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(m_native_window, &buffer, nullptr) == 0) {
        // Clear to white
        uint32_t* pixels = static_cast<uint32_t*>(buffer.bits);
        for (int y = 0; y < buffer.height; ++y) {
            for (int x = 0; x < buffer.width; ++x) {
                pixels[y * buffer.stride + x] = 0xFFFFFFFF; // white
            }
        }
        ANativeWindow_unlockAndPost(m_native_window);
    }
}

void AndroidViewImplementation::did_finish_loading(char const* url)
{
    LOGI("did_finish_loading: %s", url);
    // TODO: Notify Java layer via JNI callback
}

} // namespace Ladybird::Android
