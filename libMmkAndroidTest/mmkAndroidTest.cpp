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

#include <mmk/debug/stack.hpp>
#include <sstream>
#include <jni.h>

#define NO_INLINE   ZMMK_DEBUG_STACK_NO_INLINE // Note: ZMMK_* isn't part of the public API

extern "C" JNIEXPORT void JNICALL
Java_com_mmkAndroidTest_mmkAndroidTest_mmkDebugStackSearchPaths( JNIEnv* env, jobject thiz, jstring jElfDir )
{
	const char* elfDir  = env->GetStringUTFChars(jElfDir,  NULL);
	mmkDebugStackSearchPaths(&elfDir, 1);
	env->ReleaseStringUTFChars(jElfDir,  elfDir);
}



const char* nameOnly(const char* filename) {
	if (const char* bs = strrchr(filename, '\\')) return bs+1;
	if (const char* fs = strrchr(filename, '/'))  return fs+1;
	return filename;
}

std::stringstream lastTrace;
#define TRACE() (void)(lastTrace.str(""),   mmk::debug::stackCurrentThread(mmkDebugStackResolveAll, 0, 25,  \
    [](const mmk::debug::stackFunction& f) {                                                                \
        lastTrace << "    " << nameOnly(f.file) << "(" << f.line << "): " << f.name;                        \
        if (f.functionBase) lastTrace << " + " << (void*)((size_t)f.address - (size_t)f.functionBase);      \
        else                lastTrace << " @ " << f.address;                                                \
        lastTrace << " (" << f.module;                                                                      \
        if (f.moduleBase) lastTrace << " + " << (void*)((size_t)f.address - (size_t)f.moduleBase);          \
        lastTrace << ")\n";                                                                                 \
        return true; /* continue trace */                                                                   \
    }, MMK_DEBUG_STACK_HINT()),   0)

NO_INLINE void test_trace() { TRACE(); }
template < size_t N > struct recursive { NO_INLINE static void test_trace() { recursive<N-1>::test_trace(); } };
template <>        struct recursive<0> { NO_INLINE static void test_trace() { ::test_trace(); } };


extern "C" JNIEXPORT jstring JNICALL
Java_com_mmkAndroidTest_mmkAndroidTest_traceDemo( JNIEnv* env, jobject thiz )
{
	recursive<10>::test_trace();
	return env->NewStringUTF(lastTrace.str().c_str());
}
