package org.ladybird.androidport

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.WindowInsets
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.ime
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawing
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.union
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardActions
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.ExperimentalMaterial3Api
import androidx.compose.material3.FilledIconButton
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TopAppBar
import androidx.compose.material3.TopAppBarDefaults
import androidx.compose.material3.lightColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.platform.LocalSoftwareKeyboardController
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.ImeAction
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

/** Material 3 browser chrome; page rendering remains unavailable until LibWeb is linked. */
class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Initialize native engine (LibCore EventLoop, LibWeb bindings)
        val engineInitialized = runCatching { NativeBridge.nativeInit() }.getOrDefault(false)
        val diagnostics = runCatching { NativeBridge.platformReport() }
            .getOrElse { "Native diagnostics unavailable: ${it.message}" }
        val initialAddress = savedInstanceState?.getString("address")
            ?: getPreferences(MODE_PRIVATE).getString("address", "").orEmpty()
        setContent { LadybirdMaterialTheme { BrowserHome(diagnostics, initialAddress, engineInitialized) { address ->
            getPreferences(MODE_PRIVATE).edit().putString("address", address).apply()
        } } }
    }
}

private val md3LightScheme = lightColorScheme(
    primary = Color(0xFF355CA8), onPrimary = Color.White,
    primaryContainer = Color(0xFFD8E2FF), onPrimaryContainer = Color(0xFF001944),
    secondaryContainer = Color(0xFFDCE2F9), onSecondaryContainer = Color(0xFF151C2B),
    surface = Color(0xFFF8F9FF), surfaceContainerLow = Color(0xFFF1F3FA),
    surfaceContainer = Color(0xFFECEEFA), onSurface = Color(0xFF191B22),
    onSurfaceVariant = Color(0xFF444750), outline = Color(0xFF747783)
)

@Composable
private fun LadybirdMaterialTheme(content: @Composable () -> Unit) {
    MaterialTheme(colorScheme = md3LightScheme, content = content)
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun BrowserHome(diagnostics: String, initialAddress: String, engineLinked: Boolean, onSaveAddress: (String) -> Unit) {
    var address by rememberSaveable { mutableStateOf(initialAddress) }
    var currentUrl by rememberSaveable { mutableStateOf("") }
    val keyboard = LocalSoftwareKeyboardController.current
    val pageScroll = rememberScrollState()
    var showDiagnostics by rememberSaveable { mutableStateOf(false) }

    fun prepareNavigation() {
        val entered = address.trim()
        if (entered.isNotEmpty()) {
            address = entered
            currentUrl = if (entered.contains("://")) entered else "https://$entered"
            onSaveAddress(currentUrl)
            keyboard?.hide()
        }
    }

    Scaffold(
        contentWindowInsets = WindowInsets.safeDrawing.union(WindowInsets.ime),
        topBar = {
            TopAppBar(
                title = { Text("Ladybird", fontWeight = FontWeight.SemiBold, fontSize = 22.sp) },
                actions = {
                    IconButton(onClick = { currentUrl = ""; address = "" }) { Text("HOME", fontSize = 11.sp) }
                    IconButton(onClick = { showDiagnostics = !showDiagnostics }) { Text("INFO", fontSize = 11.sp) }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = MaterialTheme.colorScheme.surface)
            )
        },
        bottomBar = {
            Surface(color = MaterialTheme.colorScheme.surfaceContainer, tonalElevation = 2.dp, shadowElevation = 4.dp) {
                Column(
                    modifier = Modifier.fillMaxWidth().padding(horizontal = 12.dp, vertical = 10.dp),
                    verticalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    Row(
                        modifier = Modifier.fillMaxWidth(), verticalAlignment = Alignment.CenterVertically,
                        horizontalArrangement = Arrangement.spacedBy(6.dp)
                    ) {
                        IconButton(onClick = { }, enabled = false, modifier = Modifier.size(34.dp)) { Text("<") }
                        IconButton(onClick = { }, enabled = false, modifier = Modifier.size(34.dp)) { Text(">") }
                        OutlinedTextField(
                            value = address,
                            onValueChange = { address = it },
                            modifier = Modifier.weight(1f),
                            singleLine = true,
                            shape = RoundedCornerShape(28.dp),
                            placeholder = { Text("Search or enter address") },
                            leadingIcon = { Text(if (currentUrl.startsWith("https://")) "LOCK" else "WEB", fontSize = 10.sp) },
                            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Uri, imeAction = ImeAction.Go),
                            keyboardActions = KeyboardActions(onGo = { prepareNavigation() })
                        )
                        FilledIconButton(onClick = { prepareNavigation() }, modifier = Modifier.size(48.dp), shape = CircleShape) {
                            Text("GO", fontWeight = FontWeight.Bold)
                        }
                    }
                    Row(
                        modifier = Modifier.fillMaxWidth().height(30.dp),
                        horizontalArrangement = Arrangement.SpaceEvenly,
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text("BACK", color = MaterialTheme.colorScheme.onSurfaceVariant, fontSize = 10.sp)
                        Text("RELOAD", color = MaterialTheme.colorScheme.onSurfaceVariant, fontSize = 10.sp)
                        Text("HOME", color = MaterialTheme.colorScheme.onSurfaceVariant, fontSize = 10.sp)
                        Text("TABS", color = MaterialTheme.colorScheme.onSurfaceVariant, fontSize = 10.sp)
                    }
                }
            }
        }
    ) { innerPadding ->
        if (showDiagnostics) {
            // Diagnostics screen
            Column(
                modifier = Modifier.fillMaxSize().padding(innerPadding).verticalScroll(pageScroll)
                    .padding(horizontal = 20.dp, vertical = 18.dp),
                verticalArrangement = Arrangement.spacedBy(18.dp)
            ) {
                Text(
                    text = "Platform Diagnostics",
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Medium,
                    color = MaterialTheme.colorScheme.onSurface
                )
                Card(
                    modifier = Modifier.fillMaxWidth(), shape = RoundedCornerShape(24.dp),
                    colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceContainerLow)
                ) {
                    Column(modifier = Modifier.padding(18.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
                        Text("Native engine status", style = MaterialTheme.typography.titleMedium,
                            fontWeight = FontWeight.Medium)
                        Text(diagnostics, style = MaterialTheme.typography.bodySmall,
                            color = MaterialTheme.colorScheme.onSurfaceVariant)
                    }
                }
                Card(
                    modifier = Modifier.fillMaxWidth(), shape = RoundedCornerShape(28.dp),
                    colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.primaryContainer)
                ) {
                    Column(modifier = Modifier.padding(22.dp), verticalArrangement = Arrangement.spacedBy(14.dp)) {
                        Text("Android port status", style = MaterialTheme.typography.titleLarge,
                            color = MaterialTheme.colorScheme.onPrimaryContainer, fontWeight = FontWeight.SemiBold)
                        Text(
                            if (engineLinked) "Ladybird engine initialized. WebContent rendering active."
                            else "Engine not linked. Build with LADYBIRD_LINK_ENGINE=ON to enable LibWeb/LibJS.",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onPrimaryContainer
                        )
                        Text("Tap INFO again to return to browser.",
                            style = MaterialTheme.typography.labelLarge,
                            color = MaterialTheme.colorScheme.onPrimaryContainer)
                    }
                }
            }
        } else if (currentUrl.isNotEmpty()) {
            // Browser view with WebContent rendering
            LadybirdWebView(
                url = currentUrl,
                modifier = Modifier.fillMaxSize().padding(innerPadding)
            )
        } else {
            // Welcome screen
            Column(
                modifier = Modifier.fillMaxSize().padding(innerPadding).verticalScroll(pageScroll)
                    .padding(horizontal = 20.dp, vertical = 18.dp),
                verticalArrangement = Arrangement.spacedBy(18.dp)
            ) {
                Text(
                    text = "Welcome to Ladybird",
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Medium,
                    color = MaterialTheme.colorScheme.onSurface
                )
                Text(
                    text = "A new browser engine, built for the open web.",
                    style = MaterialTheme.typography.bodyLarge,
                    color = MaterialTheme.colorScheme.onSurfaceVariant
                )
                Card(
                    modifier = Modifier.fillMaxWidth(), shape = RoundedCornerShape(28.dp),
                    colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.primaryContainer)
                ) {
                    Column(modifier = Modifier.padding(22.dp), verticalArrangement = Arrangement.spacedBy(14.dp)) {
                        Text("Get started", style = MaterialTheme.typography.titleLarge,
                            color = MaterialTheme.colorScheme.onPrimaryContainer, fontWeight = FontWeight.SemiBold)
                        Text(
                            "Enter a web address in the bar above to start browsing. Try example.com or your favorite site.",
                            style = MaterialTheme.typography.bodyMedium,
                            color = MaterialTheme.colorScheme.onPrimaryContainer
                        )
                        if (engineLinked) {
                            Text("Engine: Linked and ready",
                                style = MaterialTheme.typography.labelLarge,
                                color = MaterialTheme.colorScheme.onPrimaryContainer,
                                fontWeight = FontWeight.SemiBold)
                        } else {
                            Text("Engine: Bootstrap mode (LibWeb not linked)",
                                style = MaterialTheme.typography.labelLarge,
                                color = MaterialTheme.colorScheme.onPrimaryContainer)
                        }
                    }
                }
            }
        }
    }
}
