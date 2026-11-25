// SPDX-License-Identifier: BSD-2-Clause
package org.ladybird.androidport

import android.view.Surface

object NativeBridge {
    init {
        try {
            System.loadLibrary("ladybird_android")
        } catch (e: UnsatisfiedLinkError) {
            System.loadLibrary("ladybird_android_bootstrap")
        }
    }

    // Initialize native engine once at app startup (LibCore EventLoop, LibWeb bindings)
    external fun nativeInit(): Boolean

    // Platform diagnostic report (bootstrap-compatible)
    external fun platformReport(): String

    // Create WebView instance backed by Android Surface
    external fun nativeCreateWebView(surface: Surface): Long

    // Navigate WebView to URL
    external fun nativeLoadURL(handle: Long, url: String)

    // Destroy WebView instance
    external fun nativeDestroyWebView(handle: Long)
}
