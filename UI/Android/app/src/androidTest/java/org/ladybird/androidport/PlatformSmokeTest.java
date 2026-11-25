package org.ladybird.androidport;

import static org.junit.Assert.assertTrue;

import androidx.test.ext.junit.runners.AndroidJUnit4;
import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidJUnit4.class)
public final class PlatformSmokeTest {
    @Test public void nativeStackBoundsAndLocalSocketpairWorkOnDevice() {
        String report = NativeBridge.platformReport();
        assertTrue(report, report.contains("Unix socketpair round-trip: PASS"));
        assertTrue(report, report.contains("Upstream AK::StackInfo: PASS (JNI thread)"));
        assertTrue(report, report.contains("Upstream AK::StackInfo: PASS (native worker thread)"));
        assertTrue(report, report.contains("Engine status: NOT LINKED"));
    }
}
