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

#ifndef MMK_DEBUG_STACK_USE_DBGHELP
void stack_dbghelp_exports_nothing() {} // Supress linker warning
#else

#include "dbgHelpContext.hpp"
#include "assert.hpp"

using namespace mmk::debug;
using namespace mmk::debug::dbgHelpContext;

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



//#ifndef _WIN32 // Seems we're already being skipped in 32-bit builds
//	framesToSkip += 1; // Skip self
//#endif
	MMK_DEBUG_STACK_LOCK_SCOPE();
	zmmkDebugStackInitNoLock();

	dhContext c = {};
	c.flags				= flags;
	c.process			= GetCurrentProcess();
	c.thread			= GetCurrentThread();
	c.userData			= userData;
	c.onVariable	= onVariable;

	if (!ZMMK_ASSERT(c.process, "GetCurrentProcess failed: 0x%08x", GetLastError())) return;
	if (!ZMMK_ASSERT(c.thread, "GetCurrentThread failed: 0x%08x", GetLastError())) return;
	const DWORD machineType = sizeof(void*) == 4 ? IMAGE_FILE_MACHINE_I386 : IMAGE_FILE_MACHINE_AMD64;

	c.functionContext.ContextFlags = CONTEXT_FULL; // Note: *not* CONTEXT_ALL
	RtlCaptureContext(&c.functionContext);

	c.functionDhFrame.AddrPC	.Mode = AddrModeFlat;
	c.functionDhFrame.AddrFrame	.Mode = AddrModeFlat;
	c.functionDhFrame.AddrStack	.Mode = AddrModeFlat;
#ifndef _IMAGEHLP64
	c.functionDhFrame.AddrPC	.Offset = c.functionContext.Eip;
	c.functionDhFrame.AddrFrame	.Offset = c.functionContext.Ebp;
	c.functionDhFrame.AddrStack	.Offset = c.functionContext.Esp;
#else
	c.functionDhFrame.AddrPC	.Offset = c.functionContext.Rip;
	c.functionDhFrame.AddrFrame	.Offset = c.functionContext.Rbp;
	c.functionDhFrame.AddrStack	.Offset = c.functionContext.Rsp;
#endif

	const char* const noFilename = (flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";
	const char* const noFuncName = (flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";
	const char* const noModule   = (flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";

	while (StackWalk64(machineType, c.process, c.thread, &c.functionDhFrame, &c.functionContext, NULL, NULL, NULL, NULL))
	{
		if (framesToSkip > 0) { --framesToSkip; continue; }
		if (maxFrames-- <= 0) break;

#ifndef _IMAGEHLP64
		const void* const instruction	= reinterpret_cast<const void*>(c.functionContext.Eip);
#else
		const void* const instruction	= reinterpret_cast<const void*>(c.functionContext.Rip);
#endif

		stackFunction& func = c.function;
		func.functionBase = NULL;
		func.address      = const_cast<void*>(instruction);
		func.name         = noFuncName;
		func.file         = noFilename;
		func.moduleBase   = NULL;
		func.module       = noModule;
		func.line         = 0;

		if (flags & mmkDebugStackResolveFuncName) {
			const DWORD getSymInfoErr = SymFromAddr(c.process, reinterpret_cast<size_t>(instruction), 0, &c.functionDhSymbol.info) ? 0 : GetLastError();
			func.name = getSymInfoErr ? noFilename : c.functionDhSymbol.info.Name;
			if (strcmp(func.name, __FUNCTION__)==0) continue; // Skip this frame
		}

		if (flags & (mmkDebugStackResolveFuncFile | mmkDebugStackResolveFuncLine)) {
			c.functionLine.SizeOfStruct = sizeof(c.functionLine);
			const DWORD getSymLineErr = SymGetLineFromAddr64(c.process, reinterpret_cast<size_t>(instruction), &c.functionLineDisplacement, &c.functionLine) ? 0 : GetLastError();
			func.file = getSymLineErr ? noFuncName : c.functionLine.FileName;
			func.line = getSymLineErr ? 0 : c.functionLine.LineNumber;
			if (strcmp(func.file, __FILE__)==0) continue; // Skip this frame
		}

		if (flags & mmkDebugStackResolveFuncModule) {
			// TODO: Look up module info
			func.module = noModule; // See funcSymbol.info.ModBase
		}

		if (flags & mmkDebugStackResolveFuncHints) {
			// Try to use *Hint to correct what's hopefully the deepest stack frame.
			// TODO: Some compiler flags can make __FILE__ mismatch func.file (e.g. it loses the directory prefix), add logic to handle.
			// TODO: Symbol indexing might also cause __FILE__ <-> func.file mismatches, add logic to handle.
			const bool nameMatch = func.name && funcHint && strcmp(func.name, funcHint) == 0;
			const bool fileMatch = func.file && fileHint && strcmp(func.file, fileHint) == 0;
			if (nameMatch && fileMatch && lineHint) func.line = lineHint;
		}

		const bool lastStackFrame = onFunction && !(*onFunction)(userData, &func);

		if (onVariable) {
			c.functionIhFrame.InstructionOffset = reinterpret_cast<ULONG64>(instruction);
			//if (ZMMK_ASSERT(SymSetContext(process, &ihFrame, NULL), "SymSetContext(...) failed: 0x%08x", GetLastError()))
			if (SymSetContext(c.process, &c.functionIhFrame, NULL))
			{
				//ZMMK_ASSERT(SymEnumSymbols(process, NULL, "*", &OnFrameSymbol, &c), "SymEnumSymbols(...) failed: 0x%08x", GetLastError());
				SymEnumSymbols(c.process, NULL, "*", &onFrameVar, &c);
			}
		}

		if (lastStackFrame) break;
	}
}

#endif /* def MMK_DEBUG_STACK_USE_DBGHELP */
