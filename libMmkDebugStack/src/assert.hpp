/* Copyright 2017 MaulingMonkey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef ZMMK_IG_SRC_ASSERT_HPP
#define ZMMK_IG_SRC_ASSERT_HPP

#include <stdio.h>

#ifdef _MSC_VER
#include <windows.h>
#define ZMMK_ASSERT_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#define ZMMK_BREAK_IF_DEBUGGER() (IsDebuggerPresent() ? (DebugBreak(),0) : 0)

#elif defined __ANDROID__
#include <android/log.h>
#define ZMMK_ASSERT_PRINTF(...) __android_log_print(ANDROID_LOG_ERROR, "libMmkDebugStack", __VA_ARGS__)
#define ZMMK_BREAK_IF_DEBUGGER() 0

#else
#define ZMMK_ASSERT_PRINTF(...) fprintf(stderr, __VA_ARGS__)
#define ZMMK_BREAK_IF_DEBUGGER() 0

#endif

#define ZMMK_ASSERT(condition, ...) ( (!!(condition)) || (                                           \
	ZMMK_ASSERT_PRINTF("%s(%d): ZMMK_ASSERT(\"%s\", ...) Failed: ", __FILE__, __LINE__, #condition), \
	ZMMK_ASSERT_PRINTF(__VA_ARGS__),                                                                 \
	ZMMK_ASSERT_PRINTF("\n"),                                                                        \
	ZMMK_BREAK_IF_DEBUGGER()                                                                         \
))

#endif /* ndef ZMMK_IG_SRC_ASSERT_HPP */
