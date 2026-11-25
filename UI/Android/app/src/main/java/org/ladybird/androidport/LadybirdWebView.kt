package org.ladybird.androidport

import android.view.SurfaceHolder
import android.view.SurfaceView
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.viewinterop.AndroidView

@Composable
fun LadybirdWebView(
    url: String,
    modifier: Modifier = Modifier,
    onLoadFinished: (String) -> Unit = {}
) {
    var webViewHandle by remember { mutableStateOf(0L) }

    AndroidView(
        modifier = modifier,
        factory = { context ->
            SurfaceView(context).apply {
                holder.addCallback(object : SurfaceHolder.Callback {
                    override fun surfaceCreated(holder: SurfaceHolder) {
                        // Create native WebView when Surface is ready
                        webViewHandle = NativeBridge.nativeCreateWebView(holder.surface)
                        if (webViewHandle != 0L && url.isNotEmpty()) {
                            NativeBridge.nativeLoadURL(webViewHandle, url)
                        }
                    }

                    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {
                        // TODO: Send resize event to native WebView
                    }

                    override fun surfaceDestroyed(holder: SurfaceHolder) {
                        if (webViewHandle != 0L) {
                            NativeBridge.nativeDestroyWebView(webViewHandle)
                            webViewHandle = 0L
                        }
                    }
                })
            }
        },
        update = { surfaceView ->
            // Navigate when URL changes
            if (webViewHandle != 0L && url.isNotEmpty()) {
                NativeBridge.nativeLoadURL(webViewHandle, url)
            }
        }
    )

    DisposableEffect(Unit) {
        onDispose {
            if (webViewHandle != 0L) {
                NativeBridge.nativeDestroyWebView(webViewHandle)
            }
        }
    }
}
