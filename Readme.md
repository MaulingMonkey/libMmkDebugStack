# libMmkDebugStack

A cross-platform library for simple callstack tracing with C89 and C++03 public APIs.

Requires a C++11 compiler to build.

License: [Apache 2.0](LICENSE.txt)

# Possible Use Cases
- Fancy assertion or logging macros
- Automatic tracing for bug database submission (JIRA, Sentry.IO, etc.)
- Stack traces for leaked allocations or refcounting

# Compatability

| Tested OS    | Compiler    | SDK                     | Unwind Impl              | Symbols Impl     |
| ------------ | ----------- | ----------------------- | ------------------------ | ---------------- |
| Windows 7    | VS2015 u1   |                         | DbgHelp                  | DbgHelp          |
| Android 5.0  | GCC 4.9     | nVidia Codeworks 1R4    | libunwind                | Custom elf/dwarf parser |
| Android 5.0  | Clang 3.6   | Microsoft's Android SDK | __builtin_return_address | Custom elf/dwarf parser |
| Ubuntu 16.10 | Clang 3.8.1 |                         | None (noop/fallback)     | None             |
| Ubuntu 16.10 | GCC 6.2.0   |                         | None (noop/fallback)     | None             |

Other C++11 capable platforms/compilers should at least be able to use the Fallback implementation, feel free to file an issue if they don't!

# Example

```cpp
#include <mmk/debug/stack.h> // mmkDebugStackCurrentThread etc.
#include <stdio.h>           // printf
#include <string.h>

#define NO_INLINE   ZMMK_DEBUG_STACK_NO_INLINE // Note: ZMMK_* isn't part of the public API

const char* nameOnly(const char* filename) {
	if (const char* bs = strrchr(filename, '\\')) return bs+1;
	if (const char* fs = strrchr(filename, '/'))  return fs+1;
	return filename;
}

#define TRACE() (void)(printf("Trace:\n"),   mmk::debug::stackCurrentThread(mmkDebugStackResolveAll, 0, 25, \
    [](const mmk::debug::stackFunction& f) {                                                                \
        printf("    %s(%d): %s @ %p\n", nameOnly(f.file), f.line, f.name, f.address);                       \
        return true; /* continue trace */                                                                   \
    }, MMK_DEBUG_STACK_HINT()),   printf("\n\n\n"),   0)

NO_INLINE void test_trace() { TRACE(); }
template < size_t N > struct recursive { NO_INLINE static void test_trace() { recursive<N-1>::test_trace(); } };
template <>        struct recursive<0> { NO_INLINE static void test_trace() { ::test_trace(); } };

int main()
{
	recursive<10>::test_trace();
}
```

Resulting `Debug|x86` Console:
```
Trace:
    main.cpp(19): test_trace @ 013B2C76
    main.cpp(21): recursive<0>::test_trace @ 013B2C03
    main.cpp(20): recursive<1>::test_trace @ 013B28E3
    main.cpp(20): recursive<2>::test_trace @ 013B2933
    main.cpp(20): recursive<3>::test_trace @ 013B2983
    main.cpp(20): recursive<4>::test_trace @ 013B29D3
    main.cpp(20): recursive<5>::test_trace @ 013B2A23
    main.cpp(20): recursive<6>::test_trace @ 013B2A73
    main.cpp(20): recursive<7>::test_trace @ 013B2AC3
    main.cpp(20): recursive<8>::test_trace @ 013B2B13
    main.cpp(20): recursive<9>::test_trace @ 013B2B63
    main.cpp(20): recursive<10>::test_trace @ 013B2BB3
    main.cpp(26): main @ 013B2DB3
    exe_common.inl(74): invoke_main @ 013B6B4E
    exe_common.inl(264): __scrt_common_main_seh @ 013B699A
    exe_common.inl(309): __scrt_common_main @ 013B682D
    exe_main.cpp(17): mainCRTStartup @ 013B6B68
    <unknown>(0): BaseThreadInitThunk @ 76F3336A
    <unknown>(0): RtlInitializeExceptionChain @ 77C39902
    <unknown>(0): RtlInitializeExceptionChain @ 77C398D5
```

More examples:
- mmkDebugStackTest ([c++](mmkDebugStackTest/main.cpp))
- mmkNvidiaCodeworksTest ([java](mmkNvidiaCodeworksTest/src/com/maulingmonkey/debug/stack/nvidiaCodeworksTest/DisplayStackActivity.java), [c++](mmkNvidiaCodeworksTest/jni/DisplayStackActivity.cpp))
- mmkAndroidTest ([java](mmkAndroidTest/src/com/mmkAndroidTest/mmkAndroidTest.java), [c++](libMmkAndroidTest/mmkAndroidTest.cpp))

# Installation

## NuGet
Add [libMmkDebugStack](https://www.nuget.org/packages/libMmkDebugStack/) to your project via nuget.  Done!

## From Source (on Windows development machine)
- Clone the repository
- Use `libMmkDebugStack.sln` to build `libMmkDebugStack.lib` in the configurations/platforms of your choice.
- Add `HAS_MMK_DEBUG_STACK` to your preprocessor definitions.
- Add [`libMmkDebugStack\include\`](libMmkDebugStack/include/) to your #include paths.
- Targeting Windows:
  - Add `build\native\lib\$(PlatformTarget)\$(PlatformToolset)\$(Configuration)\` to your library paths
  - Add `libMmkDebugStack.lib` and `DbgHelp.lib` to your additional dependencies list.
- Targeting Android via the nVidia Codeworks for Android 1R4 SDK:
  - Add `build\native\lib\$(ArchAbi)\$(ToolchainIdentifier)\$(Configuration)\` to your library paths
  - Add `MmkDebugStack` to your additional dependencies list.
- Targeting Android via the Microsoft Android SDK:
  - Add `build\native\lib\$(Platform)\$(PlatformToolset)\$(Configuration)\` to your library paths
  - Add `-lMmkDebugStack` to your additional dependencies list.

## From Source (on Linux development machine)
- Clone the repository
- Run make
- Add `-DHAS_MMK_DEBUG_STACK` to your `CCFLAGS`
- Add `-Lbin/$(compiler)-$(arch)-$(build)-$(language)/` to your `LDFLAGS`
- Add `-lMmkDebugStack` to your `LDFLAGS`
- Add `bin/$(compiler)-$(arch)-$(build)-$(language)/` to your `LD_LIBRARY_PATH`

# TODO
- Add option to use separate debugger process for stability / collecting stacks of foreign threads
- Backport to C++03 for wider compatability
- Properly port to Linux, OS X, iOS
- Port to Consoles, Handhelds, etc. (Blocker: Devkit access)
- Add Windows CDB backend for possible stability improvements, multi-thread stacks, crash dump analysis, etc.
