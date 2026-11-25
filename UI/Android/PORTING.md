# Android port status

Baseline: Ladybird commit `60393bb3cc9556febee314fa84a0c86b714e3368`.
Branch: `android/bringup`.

## Verified locally

- Gradle 8.9 wrapper hash matches the published Gradle SHA-256; distribution hash is pinned.
- JDK 17, Android API 35, NDK 28.0.13004108 and CMake 3.31.5 build the debug APK.
- `:app:assembleDebug :app:lintDebug --rerun-tasks` succeeds; lint XML has zero issues.
- Native bridge compiles/links for arm64-v8a and x86_64; ELF headers confirm both architectures.
- The real upstream `AK/StackInfo.cpp` compiles and is linked into each APK native library. `llvm-nm` confirms the exported constructor in both ELF files.
- JNI calls `AK::StackInfo` on the JNI entry thread and on a native worker thread, checking that a local variable falls inside the reported stack bounds.
- APK includes the JNI library and libc++ for both ABIs; Gradle Android Lint reports zero issues.

No device/emulator is currently connected, so these runtime probes have not been observed executing on Android yet. The Unix socketpair diagnostic also needs device execution. No rendered page, networking, JavaScript execution, or engine sandbox is implemented yet.

## Integration boundaries

`Libraries/LibCore/EventLoopImplementationUnix.cpp` had an Android-only branch that activated every registered notifier whenever poll returned any event, regardless of that notifier's `revents`. The branch has been removed; Android now uses the same readiness and notifier-type filtering as other Unix targets. This code change has not yet passed a full correctly configured Ladybird CMake build or an on-device pipe/socket test.

`Meta/CMake/use_linker.cmake` used `CMAKE_LINKER_TYPE=LLD`, which CMake 3.31 rejects with the Android NDK toolchain. Android now takes the existing `-fuse-ld=lld` fallback, and the root configure advances past C/C++ compiler detection.

The current Git `origin/master` tree has no `UI/Android` frontend, Android vcpkg chainload script, or Android vcpkg triplets. The Android Gradle/JNI project in this branch is an independent native smoke harness, not an upstream Ladybird UI integration. Root CMake skips host `PkgConfig` on Android and has an Android linker-type fallback. An Android vcpkg chainload and static arm64/x64 triplets are now present, and the vcpkg manifest resolver starts. The first blocker is the missing Windows host compiler: vcpkg's `x64-windows` host triplet reports `Unable to find a valid Visual Studio instance`. Attempts to install Visual Studio 2022 Build Tools via winget exited with code 87. No complete AK/LibWeb root build has succeeded.

The next build work must configure the project's actual vcpkg manifest and target toolchain rather than inventing source lists. AK needs FastFloat, simdutf, zmij, mimalloc and LLVM libc headers. Cross-build tools/code generators must run on the host; target libraries must use the NDK/Bionic sysroot.

Android app subprocess launch must be designed explicitly. The current desktop fork/exec path is not automatically a valid Android service/sandbox model. Do not run untrusted web content in the UI process merely to bypass this issue.

## Browser milestone acceptance

1. LibWeb and LibJS actually link into Android helper libraries/processes.
2. A local HTML test page renders through LibWebView and an Android surface.
3. Touch, keyboard, back navigation and Activity pause/resume work.
4. HTTPS navigation uses Ladybird networking with certificate validation.
5. Process failure is reported without killing the UI, and renderer isolation is verified.

The JNI/AK-stack-probe APK is a development artifact and does not meet these acceptance criteria.
