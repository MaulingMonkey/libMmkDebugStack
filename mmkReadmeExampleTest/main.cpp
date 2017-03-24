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
