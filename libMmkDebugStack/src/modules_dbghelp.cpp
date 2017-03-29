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
void modules_dbghelp_exports_nothing() {} // Supress linker warning
#else

#include <mmk/debug/modules.h>
#include <mmk/debug/stack.h> // zmmkDebugStackInitNoLock
#include "assert.hpp"
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable: 4091) // warning C4091: 'typedef ': ignored on left of '' when no variable is declared
#include <DbgHelp.h>
#pragma warning(pop)

namespace
{
	struct enumModulesContext
	{
		HANDLE                             process;
		mmkDebugModulesResolveFlags        flags;
		void*                              userData;
		int                                (*onModule)(void* userData,const mmkDebugModule*);
	};

	BOOL CALLBACK enumModulesCallback(
		_In_     PCSTR   ModuleName,
		_In_     DWORD64 BaseOfDll,
		_In_opt_ PVOID   UserContext)
	{
		(void)BaseOfDll;

		enumModulesContext& c = *reinterpret_cast<enumModulesContext*>(UserContext);
		const mmkDebugModulesResolveFlags flags = c.flags;


		mmkDebugModule m = {};
		if (ModuleName) m.name = ModuleName;

		IMAGEHLP_MODULE64 ihModule = {};
		ihModule.SizeOfStruct = sizeof(ihModule);
		if (flags & (mmkDebugModulesResolvePath | mmkDebugModulesResolveVersion)) { // We need the path for the version
			if (SymGetModuleInfo64(c.process, BaseOfDll, &ihModule)) {
				// Ignore ihModule.ModuleName: Likely truncated for e.g. "C:\Windows\system32\api-ms-win-core-processthreads-l1-1-1.dll"
				m.path    = ihModule.LoadedImageName;
			}
			// Lots of modules will fail to resolve info: Our exe, "KERNEL", etc.
		}

		char versionBuf[] = "99999.99999.99999.99999"; // VS_FIXEDFILEINFO is effectively encoded in shorts, 5 digits per component is thus the max
		if (m.path && (flags & mmkDebugModulesResolveVersion)) {
			char buf[1024];
			if (!GetFileVersionInfoA(m.path, 0, sizeof(buf), &buf)) {
				const DWORD err = GetLastError();
				switch (err){
				case 0:							break; // Likely just didn't have version information
				case ERROR_INSUFFICIENT_BUFFER:	break; // Version information was too big, but this is more something that should trigger a quota warning than a proper error.
				default:						ZMMK_ASSERT(err != 0, "GetFileVersionInfoA failed with GetLastError=%d", err); break;
				}
			} else {
				void* fixed = 0;
				if (ZMMK_ASSERT(VerQueryValueA(buf, "\\", &fixed, NULL) && fixed, "VerQueryValueA failed with GetLastError=%d", GetLastError())) {
					VS_FIXEDFILEINFO& ffi = *static_cast<VS_FIXEDFILEINFO*>(fixed);
					const unsigned short va = (ffi.dwFileVersionMS>>16) & 0xFFFF;
					const unsigned short vb = (ffi.dwFileVersionMS>> 0) & 0xFFFF;
					const unsigned short vc = (ffi.dwFileVersionLS>>16) & 0xFFFF;
					const unsigned short vd = (ffi.dwFileVersionLS>> 0) & 0xFFFF;
					ZMMK_ASSERT(snprintf(versionBuf, sizeof(versionBuf), "%d.%d.%d.%d", va, vb, vc, vd) < sizeof(versionBuf), "snprintf(version, ...) failed to write full version"); // Shouldn't be possible for this to fail but...
					m.version = versionBuf;
				}
			}
		}

		const bool useNull = (flags & mmkDebugModulesResolveNullMissing);
		if (!useNull) {
			if (!m.path   ) m.path    = "N/A";
			if (!m.name   ) m.name    = "N/A";
			if (!m.version) m.version = "N/A";
		}

		return (*c.onModule)(c.userData, &m) ? TRUE : FALSE;
	}
}

void mmkDebugModulesLoaded(
	mmkDebugModulesResolveFlags        flags,
	void*                              userData,
	int                                (*onModule)(void* userData,const mmkDebugModule*))
{
	MMK_DEBUG_STACK_LOCK_SCOPE();
	zmmkDebugStackInitNoLock();
	const HANDLE process = GetCurrentProcess();
	enumModulesContext c = { process, flags, userData, onModule };
	ZMMK_ASSERT(SymRefreshModuleList(process), "SymRefreshModuleList failed with GetLastError=%d", GetLastError());
	ZMMK_ASSERT(SymEnumerateModules64(process, &enumModulesCallback, &c), "SymEnumerateModules64 failed with GetLastError=%d", GetLastError());
}


#endif /* def MMK_DEBUG_STACK_USE_DBGHELP */
