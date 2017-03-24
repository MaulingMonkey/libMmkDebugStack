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
void init_dbghelp_exports_nothing() {} // Supress linker warning
#else

#include <mmk/debug/stack.hpp>
#include "assert.hpp"

#include <windows.h>
//#include <cvconst.h>
#define _NO_CVCONST_H // DbgHelp will define some of the constants cvconst.h provides, since cvconst.h isn't available in my source tree.
#pragma warning(push)
#pragma warning(disable: 4091) // warning C4091: 'typedef ': ignored on left of '' when no variable is declared
#include <DbgHelp.h>
#pragma warning(pop)

namespace
{
	const char* const	libraryAtomId = "{EC7FF5FB-05A8-4345-9C99-C637C2D7B625}"; // Arbitrary GUID used to detect double-linking the same library.
	bool				libraryInitialized = false;

	void mmkDebugStackInit_LocalDbgHelp()
	{
		if (libraryInitialized) return; // Library was already initialized, don't double-init
		libraryInitialized = true;

		const ATOM foundAtom = FindAtomA(libraryAtomId);
		if (!ZMMK_ASSERT(!foundAtom, "mmkDebugStack double initialized despite protects:  You may be linking this library twice!  This will cause thread safety issues.")) return;
		const ATOM newAtom = AddAtomA(libraryAtomId);
		ZMMK_ASSERT(  newAtom, "AddAtom(\"%s\") failed", libraryAtomId);

		const HANDLE proc = GetCurrentProcess();
		if (!ZMMK_ASSERT(SymInitialize(proc, NULL, TRUE), "SymInitialize(%p, NULL, TRUE) failed: mmkDebugStack will likely malfunction", proc)) return;

		const DWORD originalOptions = SymGetOptions();
		const DWORD newOptions = SYMOPT_DEBUG | SYMOPT_NO_PROMPTS | SYMOPT_LOAD_LINES;
		SymSetOptions(originalOptions | newOptions); // Returns 'current' (previous? new?) mask, not an error code.
	}
}

void mmkDebugStackInit(void)
{
	MMK_DEBUG_STACK_LOCK_SCOPE();
	zmmkDebugStackInitNoLock();
}

void zmmkDebugStackInitNoLock(void)
{
	mmkDebugStackInit_LocalDbgHelp();
}

#endif /* def MMK_DEBUG_STACK_USE_DBGHELP */
