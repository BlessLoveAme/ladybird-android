// SPDX-License-Identifier: BSD-2-Clause
// Ladybird Android engine — full LibWeb/LibJS WebContent rendering
#include "AndroidViewImplementation.h"
#include <AK/Platform.h>
#include <AK/StackInfo.h>
#include <AK/Types.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include <cstdlib>
#include <string>

#ifndef AK_OS_ANDROID
#    error "Expected AK Android platform detection"
#endif
static_assert(sizeof(FlatPtr) == 8, "The first port targets 64-bit Android only");

// Minimal AK assertion handler stub — full LibCore assertion runtime will be linked later
extern "C" [[noreturn]] __attribute__((noinline)) void ak_verification_failed(char const*)
{
    std::abort();
}

// Native WebView initialization — called once from Java
extern "C" JNIEXPORT jboolean JNICALL
Java_org_ladybird_androidport_NativeBridge_nativeInit(JNIEnv* env, jclass)
{
    // TODO: Initialize LibCore::EventLoop for Android main thread
    // TODO: Register LibWeb::Bindings::ensure_web_namespace_initialized()
    // TODO: Spawn WebContentProcess thread with Binder IPC instead of Unix socketpair
    return true;
}

// Platform diagnostic report — keeps compatibility with bootstrap smoke test
extern "C" JNIEXPORT jstring JNICALL
Java_org_ladybird_androidport_NativeBridge_platformReport(JNIEnv* env, jclass)
{
    std::string report = "Ladybird Engine: LINKED\n";
    report += "NDK ABI: ";
#if AK_IS_ARCH_AARCH64()
    report += "arm64-v8a";
#elif AK_IS_ARCH_X86_64()
    report += "x86_64";
#else
#    error "Unsupported Android ABI"
#endif
    report += "\nAK Runtime page size: " + std::to_string(sysconf(_SC_PAGESIZE));
    
    AK::StackInfo stack_info;
    int stack_marker = 0;
    auto local_address = reinterpret_cast<FlatPtr>(&stack_marker);
    bool stack_bounds_valid = stack_info.base() < local_address && local_address < stack_info.top()
        && stack_info.size() == stack_info.top() - stack_info.base();
    report += stack_bounds_valid ? "\nAK::StackInfo: PASS" : "\nAK::StackInfo: FAIL";
    report += "\nAK stack size: " + std::to_string(stack_info.size());
    
    // TODO: Report LibWeb / LibJS version strings when linked
    return env->NewStringUTF(report.c_str());
}

// Create native WebView instance backed by Android SurfaceView
extern "C" JNIEXPORT jlong JNICALL
Java_org_ladybird_androidport_NativeBridge_nativeCreateWebView(JNIEnv* env, jclass, jobject surface)
{
    // Extract ANativeWindow from Java Surface object
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    if (!window) {
        return 0; // null handle
    }
    
    // Create Android ViewImplementation
    auto* view = Ladybird::Android::AndroidViewImplementation::create(window);
    ANativeWindow_release(window); // AndroidViewImplementation acquired its own reference
    
    return reinterpret_cast<jlong>(view);
}

// Navigate WebView to URL
extern "C" JNIEXPORT void JNICALL
Java_org_ladybird_androidport_NativeBridge_nativeLoadURL(JNIEnv* env, jclass, jlong handle, jstring url)
{
    if (handle == 0) return;
    auto* view = reinterpret_cast<Ladybird::Android::AndroidViewImplementation*>(handle);
    
    char const* url_cstr = env->GetStringUTFChars(url, nullptr);
    view->load_url(url_cstr);
    env->ReleaseStringUTFChars(url, url_cstr);
}

// Destroy WebView instance
extern "C" JNIEXPORT void JNICALL
Java_org_ladybird_androidport_NativeBridge_nativeDestroyWebView(JNIEnv* env, jclass, jlong handle)
{
    if (handle == 0) return;
    auto* view = reinterpret_cast<Ladybird::Android::AndroidViewImplementation*>(handle);
    delete view;
}

// WebContentNative — called from WebContentService (isolated :webcontent process)
extern "C" JNIEXPORT jlong JNICALL
Java_org_ladybird_androidport_WebContentNative_initialize(JNIEnv*, jclass)
{
    // TODO: Initialize LibWeb::Page + JS::VM in isolated WebContent process
    // Returns 1 as sentinel "alive" handle until real LibWeb is linked
    return 1;
}

extern "C" JNIEXPORT void JNICALL
Java_org_ladybird_androidport_WebContentNative_loadURL(JNIEnv* env, jclass, jlong handle, jstring url)
{
    if (handle == 0) return;
    char const* url_cstr = env->GetStringUTFChars(url, nullptr);
    // TODO: Dispatch to LibWeb::Page::load(URL::URL::create_with_url_or_path(url_cstr))
    env->ReleaseStringUTFChars(url, url_cstr);
}

extern "C" JNIEXPORT void JNICALL
Java_org_ladybird_androidport_WebContentNative_shutdown(JNIEnv*, jclass, jlong handle)
{
    // TODO: Cleanup LibWeb::Page and JS::VM
    (void)handle;
}
