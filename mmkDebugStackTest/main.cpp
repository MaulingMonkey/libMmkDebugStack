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
#include <stddef.h>          // ptrdiff_t



// Note: ZMMK_* isn't part of the public API and may be subject to change.
#define NO_INLINE ZMMK_DEBUG_STACK_NO_INLINE



// -------------- Simple tracing macro: C++ API version (lambda & functor capable) --------------
#define TRACE1() (void)(printf("Trace #1:\n"),   mmk::debug::stackCurrentThread(mmkDebugStackResolveAll, 0, 25, [](const mmk::debug::stackFunction& f) { \
	printf("    %s(%d): %s @ %p\n", f.file, f.line, f.name, f.address);                                                                                  \
	return true;                                                                                                                                         \
}, MMK_DEBUG_STACK_HINT()),   printf("\n\n\n"),   0)



// -------------- Simple tracing macro: C++ API version (lambda & functor capable) --------------
#define TRACE2() (void)(printf("Trace #2:\n"),   mmk::debug::stackCurrentThread(mmkDebugStackResolveAll, 0, 25, [](const mmk::debug::stackFunction& f) { \
    if (!needsHeader) printf("        -----------------------------------------------------------------------------\n");                                 \
    needsHeader = true;                                                                                                                                  \
    printf("    %s(%d): %s @ %p\n", f.file, f.line, f.name, f.address);                                                                                  \
    return true;                                                                                                                                         \
}, [](const mmk::debug::stackVariable& v) {                                                                                                              \
    if (needsHeader)                                                                                                                                     \
    {                                                                                                                                                    \
        printf("        -----------------------------------------------------------------------------\n");                                               \
        printf("        %-20s %-10s  %20s  %s\n", "name", "value", "type", "address");                                                                   \
        printf("        -----------------------------------------------------------------------------\n");                                               \
    }                                                                                                                                                    \
    printf("        %-20s %-10s (%20s) %p\n", v.name, v.value, v.type, v.address);                                                                       \
    needsHeader = false;                                                                                                                                 \
    return true;                                                                                                                                         \
}, MMK_DEBUG_STACK_HINT()),   printf("\n\n\n"),   0)



// -------------- Simple tracing macro: C API version --------------
#define TRACE3() (void)(printf("Trace #3:\n"),   mmkDebugStackCurrentThread(mmkDebugStackResolveAll, 0, 25, NULL, &traceFunction, &traceVariable, MMK_DEBUG_STACK_HINT()),   printf("\n\n\n"),   0)
bool needsHeader = true;
int traceFunction(void*, const mmkDebugStackFunction* f)
{
	if (!needsHeader) printf("        -----------------------------------------------------------------------------\n");
	needsHeader = true;
	printf("    %s(%d): %s @ %p\n", f->file, f->line, f->name, f->address);
	return 1; // Continue
}
int traceVariable(void*, const mmkDebugStackVariable* v)
{
	if (needsHeader)
	{
		printf("        -----------------------------------------------------------------------------\n");
		printf("        %-20s %-10s  %20s  %s\n", "name", "value", "type", "address");
		printf("        -----------------------------------------------------------------------------\n");
	}
	printf("        %-20s %-10s (%20s) %p\n", v->name, v->value, v->type, v->address);
	needsHeader = false;
	// Also available: v->function->{file, line, name, address}
	return 1; // Continue
}



// -------------- Demo using tracing macros --------------
NO_INLINE void test_trace() { TRACE1(); TRACE2(); TRACE3(); }
template < size_t N > struct recursive    { NO_INLINE static void test_trace() { recursive<N-1>::test_trace(); } };
template <>           struct recursive<0> { NO_INLINE static void test_trace() { ::test_trace(); } };

struct foo { int i; };
typedef foo bar;

int main()
{
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4189) // local variable is initialized but not referenced
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable" // error: unused variable 'aWChar' [-Werror=unused-variable]
#endif

	char                aChar      = 'A';
	wchar_t             aWChar     = L'A';
	char                aStr[]     = "Alas, poor yorrik";
	wchar_t             aWstr[]    = L"Alas, poor yorrik";
	const char         *aPStr      = "Alas, poor yorrik";
	const wchar_t      *aPWstr     = L"Alas, poor yorrik";

	signed char         aSChar     = 42;
	signed short        aSShort    = 42;
	signed int          aSInt      = 42;
	signed long         aSLong     = 42;
	signed long long    aSLongLong = 42;

	unsigned char       aUChar     = 42;
	unsigned short      aUShort    = 42;
	unsigned int        aUInt      = 42;
	unsigned long       aULong     = 42;
	unsigned long long  aULongLong = 42;

	size_t              aSize      = 42;
	ptrdiff_t           aDiff      = 42;

	foo                 aFoo       = { 42 };
	bar                 aBar       = { 42 };

	void*               aPNull     = NULL;
	void*               aPStack    = &aChar;

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined __GNUC__
#pragma GCC diagnostic pop
#endif

	recursive<10>::test_trace();
}
