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

#if defined(MMK_DEBUG_STACK_USE_DBGHELP) || defined(MMK_DEBUG_STACK_USE_DLADDR)
void stack_fallback_exports_nothing() {} // Supress linker warning
#else

#include <mmk/debug/stack.hpp>
#include "assert.hpp"

using namespace mmk::debug;

// Enumerating the callstack absolutely requires platform specific support, which we lack.
// As a token effort, we can at least emit a single "function" based on {file,line,func}Hint, if available.
// If not even that is available, we'll still emit a single "I know nothing!" frame.
ZMMK_DEBUG_STACK_API
ZMMK_DEBUG_STACK_NO_INLINE
void mmkDebugStackCurrentThread(
	mmkDebugStackResolveFlags	flags,
	unsigned					framesToSkip,
	unsigned					maxFrames,
	void*						userData,
	int							(*onFunction) (void* userData, const mmkDebugStackFunction*),
	int							(*onVariable) (void* userData, const mmkDebugStackVariable*),
	const char*					fileHint,
	unsigned					lineHint,
	const char*					funcHint)
{
	// Preconditions

	// framesToSkip may be 0
	if (!ZMMK_ASSERT(maxFrames > 0, "Expected maxFrames > 0")) return;
	// flags may be 0
	// userData may be NULL
	if (!ZMMK_ASSERT(onFunction, "Expected onFunction callback")) return;
	// onVariable may be NULL
	// fileHint may be NULL
	// lineHint may be 0
	// funcHint may be NULL



	// This fallback version can't use most info.
	(void)framesToSkip;
	(void)maxFrames;
	(void)onVariable;

	if (!funcHint && !fileHint && !lineHint) return; // The only frame we could possibly return is the fake hint based one, and no hint was given.

	const char* const noFuncFile     = (flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";
	const char* const noFuncName     = (flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";
	const char* const noFuncModule   = (flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";

	stackFunction f = {};
	f.address = NULL;
	f.name    = funcHint ? funcHint : noFuncName;
	f.file    = fileHint ? fileHint : noFuncFile;
	f.line    = lineHint ? lineHint : 0;
	f.module  = noFuncModule;

	(void)(*onFunction)(userData, &f);
}

#endif /* !defined(...) */
