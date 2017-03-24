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

#ifndef ZMMK_IG_SRC_DBGHELPCONTEXT_HPP
#define ZMMK_IG_SRC_DBGHELPCONTEXT_HPP
#ifdef MMK_DEBUG_STACK_USE_DBGHELP

#include <mmk/debug/stack.h>

#include <windows.h>
//#include <cvconst.h>
#define _NO_CVCONST_H // DbgHelp will define some of the constants cvconst.h provides, since cvconst.h isn't available in my source tree.
#pragma warning(push)
#pragma warning(disable: 4091) // warning C4091: 'typedef ': ignored on left of '' when no variable is declared
#include <DbgHelp.h>
#pragma warning(pop)

namespace mmk { namespace debug { namespace dbgHelpContext {
	struct dhSymbolInfo {
		SYMBOL_INFO		info;
		char			name[1024];

		dhSymbolInfo()
			: info()
			, name()
		{
			info.MaxNameLen = sizeof(name)-1;
			info.SizeOfStruct = sizeof(info);
		}
	};

	// TODO: Break up to make this a bit harder to mis-initialize or otherwise misuse.
	struct dhContext {
		// Semi-constant for duration of mmkDebugStackCurrentThread
		mmkDebugStackResolveFlags	flags;
		HANDLE						process;
		HANDLE						thread;
		void*						userData;
		int							(*onVariable)	(void* userData, const mmkDebugStackVariable*);

		// Mutated during iteration of mmkDebugStackCurrentThread
		CONTEXT						functionContext;
		STACKFRAME64				functionDhFrame;
		dhSymbolInfo				functionDhSymbol;
		DWORD						functionLineDisplacement;
		IMAGEHLP_LINE64				functionLine;
		IMAGEHLP_STACK_FRAME		functionIhFrame;
		stackFunction				function;

		// Mutated during iteration of OnFrameSymbol
		stackVariable				var;
		PSYMBOL_INFO				varDhSymbol;
		ULONG						varSizeof;

		dhContext()
			: flags()
			, process()
			, thread()
			, userData()
			, onVariable()

			, functionContext()
			, functionDhFrame()
			, functionDhSymbol()
			, functionLineDisplacement()
			, functionLine()
			, functionIhFrame()
			, function()

			, var()
			, varDhSymbol()
			, varSizeof()
		{
		}
	};

	// dbgHelpContext_onFrameVar.cpp
	BOOL CALLBACK onFrameVar(PSYMBOL_INFO pSymInfo, ULONG nSymSize, PVOID pContext);

	// dbgHelpContext_baseType.cpp
	const char* baseTypeName(DWORD type, DWORD size);
	bool onFrameVar_BaseType(dhContext& c);

}}} // namespace mmk::debug::dbgHelpContext

#endif /* def MMK_DEBUG_STACK_USE_DBGHELP */ 
#endif /* ndef ZMMK_IG_SRC_DBGHELPCONTEXT_HPP */
