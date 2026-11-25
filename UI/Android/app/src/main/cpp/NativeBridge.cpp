// SPDX-License-Identifier: BSD-2-Clause
#include <AK/Platform.h>
#include <AK/StackInfo.h>
#include <AK/Types.h>
#include <jni.h>
#include <cstdlib>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <thread>

#ifndef AK_OS_ANDROID
#    error "Expected AK Android platform detection"
#endif
static_assert(sizeof(FlatPtr) == 8, "The first port targets 64-bit Android only");

// StackInfo calls this upstream AK assertion hook if pthread fails; full AK assertion runtime is not linked in this probe.
extern "C" [[noreturn]] __attribute__((noinline)) void ak_verification_failed(char const*)
{
    std::abort();
}

extern "C" JNIEXPORT jstring JNICALL
Java_org_ladybird_androidport_NativeBridge_platformReport(JNIEnv* env, jclass)
{
    int sockets[2];
    bool ipc_ok = false;
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sockets) == 0) {
        char sent = 'L';
        char received = 0;
        ipc_ok = send(sockets[0], &sent, 1, MSG_NOSIGNAL) == 1
            && recv(sockets[1], &received, 1, MSG_DONTWAIT) == 1 && received == sent;
        close(sockets[0]);
        close(sockets[1]);
    }
    std::string report = "NDK JNI library: loaded\nAK Android headers: compiled\nABI: ";
#if AK_IS_ARCH_AARCH64()
    report += "arm64-v8a";
#elif AK_IS_ARCH_X86_64()
    report += "x86_64";
#else
#    error "Unsupported Android ABI"
#endif
    report += "\nRuntime page size: " + std::to_string(sysconf(_SC_PAGESIZE));
    report += ipc_ok ? "\nUnix socketpair round-trip: PASS" : "\nUnix socketpair round-trip: FAIL";
    AK::StackInfo stack_info;
    int stack_marker = 0;
    auto local_address = reinterpret_cast<FlatPtr>(&stack_marker);
    bool stack_bounds_valid = stack_info.base() < local_address && local_address < stack_info.top()
        && stack_info.size() == stack_info.top() - stack_info.base();
    report += stack_bounds_valid ? "\nUpstream AK::StackInfo: PASS (JNI thread)" : "\nUpstream AK::StackInfo: FAIL";
    report += "\nAK stack size: " + std::to_string(stack_info.size());

    bool worker_stack_bounds_valid = false;
    std::thread worker([&] {
        AK::StackInfo worker_stack_info;
        int worker_stack_marker = 0;
        auto worker_local_address = reinterpret_cast<FlatPtr>(&worker_stack_marker);
        worker_stack_bounds_valid = worker_stack_info.base() < worker_local_address
            && worker_local_address < worker_stack_info.top()
            && worker_stack_info.size() == worker_stack_info.top() - worker_stack_info.base();
    });
    worker.join();
    report += worker_stack_bounds_valid ? "\nUpstream AK::StackInfo: PASS (native worker thread)"
                                        : "\nUpstream AK::StackInfo: FAIL (native worker thread)";
    report += "\nEngine status: NOT LINKED";
    return env->NewStringUTF(report.c_str());
}
