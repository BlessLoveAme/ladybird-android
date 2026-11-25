package org.ladybird.androidport

import android.app.Service
import android.content.Intent
import android.os.IBinder
import android.util.Log

/**
 * WebContent rendering service — isolated process for LibWeb/LibJS rendering.
 *
 * Runs in a separate process (:webcontent) for renderer isolation.
 * Communicates with UI process via Android Binder IPC.
 *
 * This mirrors the upstream Ladybird multi-process architecture where
 * WebContent runs as an isolated helper process communicating via IPC.
 */
class WebContentService : Service() {
    companion object {
        private const val TAG = "WebContentService"

        const val ACTION_LOAD_URL = "org.ladybird.androidport.LOAD_URL"
        const val EXTRA_URL = "url"
        const val EXTRA_VIEW_HANDLE = "view_handle"
    }

    private var nativeHandle: Long = 0

    override fun onCreate() {
        super.onCreate()
        Log.i(TAG, "WebContentService created")
        // Initialize native WebContent engine on service startup
        nativeHandle = WebContentNative.initialize()
        if (nativeHandle == 0L) {
            Log.e(TAG, "Failed to initialize native WebContent engine")
        }
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        intent?.let { handleCommand(it) }
        return START_STICKY
    }

    override fun onBind(intent: Intent?): IBinder? {
        // TODO: Return Binder for two-way IPC with UI process
        return null
    }

    override fun onDestroy() {
        super.onDestroy()
        Log.i(TAG, "WebContentService destroyed")
        if (nativeHandle != 0L) {
            WebContentNative.shutdown(nativeHandle)
            nativeHandle = 0L
        }
    }

    private fun handleCommand(intent: Intent) {
        when (intent.action) {
            ACTION_LOAD_URL -> {
                val url = intent.getStringExtra(EXTRA_URL) ?: return
                Log.i(TAG, "LoadURL: $url")
                if (nativeHandle != 0L) {
                    WebContentNative.loadURL(nativeHandle, url)
                }
            }
        }
    }
}

/** JNI bridge for WebContent service (native LibWeb/LibJS calls) */
object WebContentNative {
    init {
        try {
            System.loadLibrary("ladybird_android")
        } catch (e: UnsatisfiedLinkError) {
            System.loadLibrary("ladybird_android_bootstrap")
        }
    }

    // Initialize WebContent engine — returns opaque handle or 0 on failure
    external fun initialize(): Long

    // Load URL into WebContent
    external fun loadURL(handle: Long, url: String)

    // Shutdown WebContent engine
    external fun shutdown(handle: Long)
}
