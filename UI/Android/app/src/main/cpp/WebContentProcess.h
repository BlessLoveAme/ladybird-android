// SPDX-License-Identifier: BSD-2-Clause
// Android WebContent process — renders web pages in isolated thread
#pragma once

#include <AK/Types.h>
#include <pthread.h>

namespace Ladybird::Android {

class AndroidViewImplementation;

// WebContent rendering process — runs LibWeb/LibJS in dedicated thread
// Communicates with UI thread via IPC (Unix pipe on Android)
class WebContentProcess {
public:
    static WebContentProcess* create(AndroidViewImplementation* view);
    ~WebContentProcess();

    // IPC commands from UI thread
    void load_url(char const* url);
    void handle_resize(int width, int height);
    void handle_mouse_event(int x, int y, int button, bool pressed);
    void handle_scroll_event(float delta_x, float delta_y);

    // Check if process is alive
    bool is_running() const { return m_running; }

private:
    WebContentProcess(AndroidViewImplementation* view);
    bool initialize();
    
    // Main WebContent thread loop
    static void* thread_main(void* arg);
    void run();

    // IPC message handling
    void process_ipc_messages();
    void send_paint_request();

    AndroidViewImplementation* m_view { nullptr };
    pthread_t m_thread {};
    int m_ipc_fds[2] { -1, -1 }; // Unix pipe for IPC
    bool m_running { false };
    void* m_web_page { nullptr }; // LibWeb::Page* (opaque until linked)
};

} // namespace Ladybird::Android
