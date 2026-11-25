// SPDX-License-Identifier: BSD-2-Clause
#include "WebContentProcess.h"
#include "AndroidViewImplementation.h"
#include "LibWebAndroid.h"
#include <android/log.h>
#include <unistd.h>
#include <poll.h>
#include <cstring>
#include <cerrno>

#define LOG_TAG "WebContent"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace Ladybird::Android {

// IPC message types
enum class IPCMessageType : u8 {
    LoadURL = 1,
    Resize = 2,
    MouseEvent = 3,
    ScrollEvent = 4,
    PaintRequest = 5,
};

struct IPCMessage {
    IPCMessageType type;
    union {
        struct { char url[512]; } load_url;
        struct { int width; int height; } resize;
        struct { int x; int y; int button; bool pressed; } mouse;
        struct { float delta_x; float delta_y; } scroll;
    } data;
};

WebContentProcess* WebContentProcess::create(AndroidViewImplementation* view)
{
    auto* process = new WebContentProcess(view);
    if (!process->initialize()) {
        delete process;
        return nullptr;
    }
    return process;
}

WebContentProcess::WebContentProcess(AndroidViewImplementation* view)
    : m_view(view)
{
}

WebContentProcess::~WebContentProcess()
{
    m_running = false;
    if (m_thread) {
        pthread_join(m_thread, nullptr);
        m_thread = 0;
    }
    if (m_ipc_fds[0] >= 0) close(m_ipc_fds[0]);
    if (m_ipc_fds[1] >= 0) close(m_ipc_fds[1]);
}

bool WebContentProcess::initialize()
{
    // Create Unix pipe for IPC (Android supports this, unlike fork+exec)
    if (pipe(m_ipc_fds) < 0) {
        LOGE("Failed to create IPC pipe: %s", strerror(errno));
        return false;
    }

    // Spawn WebContent thread
    m_running = true;
    int result = pthread_create(&m_thread, nullptr, &WebContentProcess::thread_main, this);
    if (result != 0) {
        LOGE("Failed to create WebContent thread: %s", strerror(result));
        m_running = false;
        return false;
    }

    LOGI("WebContent process initialized (thread %lu)", m_thread);
    return true;
}

void* WebContentProcess::thread_main(void* arg)
{
    auto* process = static_cast<WebContentProcess*>(arg);
    process->run();
    return nullptr;
}

void WebContentProcess::run()
{
    LOGI("WebContent thread started");

    // Initialize LibWeb for this thread
    if (!initialize_libweb_for_android()) {
        LOGE("Failed to initialize LibWeb — rendering disabled");
        // Continue anyway for IPC testing, but no actual rendering
    }

    // Create LibWeb::Page with initial viewport size
    int width = 800, height = 600; // Default, will be updated by Resize IPC
    m_web_page = create_web_page(width, height);
    if (!m_web_page) {
        LOGE("Failed to create LibWeb::Page — rendering disabled");
    }

    // Main event loop — poll IPC pipe for commands from UI thread
    struct pollfd pfd;
    pfd.fd = m_ipc_fds[0]; // read end
    pfd.events = POLLIN;

    while (m_running) {
        int ret = poll(&pfd, 1, 100); // 100ms timeout
        if (ret < 0) {
            if (errno == EINTR) continue;
            LOGE("poll() failed: %s", strerror(errno));
            break;
        }
        if (ret > 0 && (pfd.revents & POLLIN)) {
            process_ipc_messages();
        }

        // TODO: Process LibCore::EventLoop timers/tasks
        // TODO: Handle JS microtasks
    }

    // Cleanup LibWeb::Page
    if (m_web_page) {
        destroy_web_page(m_web_page);
        m_web_page = nullptr;
    }
    shutdown_libweb();

    LOGI("WebContent thread exiting");
}

void WebContentProcess::process_ipc_messages()
{
    IPCMessage msg;
    ssize_t n = read(m_ipc_fds[0], &msg, sizeof(msg));
    if (n != sizeof(msg)) {
        if (n < 0 && errno != EAGAIN) {
            LOGE("IPC read failed: %s", strerror(errno));
        }
        return;
    }

    switch (msg.type) {
    case IPCMessageType::LoadURL:
        LOGI("LoadURL IPC: %s", msg.data.load_url.url);
        if (m_web_page) {
            web_page_load_url(m_web_page, msg.data.load_url.url);
        }
        send_paint_request();
        break;

    case IPCMessageType::Resize:
        LOGI("Resize IPC: %dx%d", msg.data.resize.width, msg.data.resize.height);
        if (m_web_page) {
            web_page_resize(m_web_page, msg.data.resize.width, msg.data.resize.height);
        }
        send_paint_request();
        break;

    case IPCMessageType::MouseEvent:
        if (m_web_page) {
            web_page_mouse_event(m_web_page, msg.data.mouse.x, msg.data.mouse.y,
                msg.data.mouse.button, msg.data.mouse.pressed);
        }
        break;

    case IPCMessageType::ScrollEvent:
        // TODO: Call LibWeb::Page::handle_scroll()
        send_paint_request();
        break;

    default:
        LOGE("Unknown IPC message type: %d", static_cast<int>(msg.type));
        break;
    }
}

void WebContentProcess::send_paint_request()
{
    // Render LibWeb::Page to frame buffer
    if (m_web_page) {
        [[maybe_unused]] void* frame_buffer = web_page_render(m_web_page);
        // TODO: Pass frame_buffer to AndroidViewImplementation compositor
    }

    // Request UI thread to repaint ANativeWindow
    if (m_view) {
        m_view->request_repaint();
    }
}

void WebContentProcess::load_url(char const* url)
{
    IPCMessage msg;
    msg.type = IPCMessageType::LoadURL;
    strncpy(msg.data.load_url.url, url, sizeof(msg.data.load_url.url) - 1);
    msg.data.load_url.url[sizeof(msg.data.load_url.url) - 1] = '\0';

    ssize_t n = write(m_ipc_fds[1], &msg, sizeof(msg));
    if (n != sizeof(msg)) {
        LOGE("Failed to send LoadURL IPC: %s", strerror(errno));
    }
}

void WebContentProcess::handle_resize(int width, int height)
{
    IPCMessage msg;
    msg.type = IPCMessageType::Resize;
    msg.data.resize.width = width;
    msg.data.resize.height = height;

    write(m_ipc_fds[1], &msg, sizeof(msg));
}

void WebContentProcess::handle_mouse_event(int x, int y, int button, bool pressed)
{
    IPCMessage msg;
    msg.type = IPCMessageType::MouseEvent;
    msg.data.mouse.x = x;
    msg.data.mouse.y = y;
    msg.data.mouse.button = button;
    msg.data.mouse.pressed = pressed;

    write(m_ipc_fds[1], &msg, sizeof(msg));
}

void WebContentProcess::handle_scroll_event(float delta_x, float delta_y)
{
    IPCMessage msg;
    msg.type = IPCMessageType::ScrollEvent;
    msg.data.scroll.delta_x = delta_x;
    msg.data.scroll.delta_y = delta_y;

    write(m_ipc_fds[1], &msg, sizeof(msg));
}

} // namespace Ladybird::Android
