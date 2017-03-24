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
void dbgHelpContext_onFrameVar_exports_nothing() {} // Supress linker warning
#else
#include "dbgHelpContext.hpp"
#include "assert.hpp"

namespace mmk { namespace debug { namespace dbgHelpContext {

	// TODO: Implement all these.
	void onFrameVar_Pointer         (dhContext& c) { (void)c; }
	void onFrameVar_Array           (dhContext& c) { (void)c; }
	void onFrameVar_UserDefinedType (dhContext& c) { (void)c; }
	void onFrameVar_Enum            (dhContext& c) { (void)c; }
	void onFrameVar_UnknownType     (dhContext& c) { (void)c; if (IsDebuggerPresent()) DebugBreak(); }

	BOOL CALLBACK onFrameVar(PSYMBOL_INFO pSymInfo, ULONG nSymSize, PVOID pContext) {
		if (!ZMMK_ASSERT(pSymInfo, "OnFrameSymbol: pSymInfo null!")) return TRUE; // BUG?
		if (!ZMMK_ASSERT(pContext, "OnFrameSymbol: pContext null!")) return TRUE; // BUG!

		auto& c = *reinterpret_cast<dhContext*>(pContext);

		const char* const noName  = (c.flags & mmkDebugStackResolveNullMissing) ? NULL : "???";
		const char* const noType  = (c.flags & mmkDebugStackResolveNullMissing) ? NULL : "???";
		const char* const noValue = (c.flags & mmkDebugStackResolveNullMissing) ? NULL : "...";

		c.var.function = &c.function;
		c.var.address  = NULL;
		c.var.name     = pSymInfo ? pSymInfo->Name : noName;
		c.var.value    = noValue;
		c.var.type     = noType;
		c.varDhSymbol  = pSymInfo;
		c.varSizeof    = nSymSize;

		if (c.flags & (mmkDebugStackResolveVarType | mmkDebugStackResolveVarValue))
		{
			DWORD tag = 0;
			if (ZMMK_ASSERT(SymGetTypeInfo(c.process, c.varDhSymbol->ModBase, c.varDhSymbol->TypeIndex, TI_GET_SYMTAG, &tag), "SymGetTypeInfo(...) failed: 0x%08x", GetLastError())) switch (tag)
			{
			case SymTagUDT:					onFrameVar_UserDefinedType(c); break;
			case SymTagEnum:				onFrameVar_Enum(c);            break;
			case SymTagPointerType:			onFrameVar_Pointer(c);         break;
			case SymTagArrayType:			onFrameVar_Array(c);           break;
			case SymTagBaseType:			return onFrameVar_BaseType(c);

			case SymTagNull:				onFrameVar_UnknownType(c);     break;
			case SymTagExe:					onFrameVar_UnknownType(c);     break;
			case SymTagCompiland:			onFrameVar_UnknownType(c);     break;
			case SymTagCompilandDetails:	onFrameVar_UnknownType(c);     break;
			case SymTagCompilandEnv:		onFrameVar_UnknownType(c);     break;
			case SymTagFunction:			onFrameVar_UnknownType(c);     break;
			case SymTagBlock:				onFrameVar_UnknownType(c);     break;
			case SymTagData:				onFrameVar_UnknownType(c);     break;
			case SymTagAnnotation:			onFrameVar_UnknownType(c);     break;
			case SymTagLabel:				onFrameVar_UnknownType(c);     break;
			case SymTagPublicSymbol:		onFrameVar_UnknownType(c);     break;
			case SymTagFunctionType:		onFrameVar_UnknownType(c);     break;
			case SymTagTypedef:				onFrameVar_UnknownType(c);     break;
			case SymTagBaseClass:			onFrameVar_UnknownType(c);     break;
			case SymTagFriend:				onFrameVar_UnknownType(c);     break;
			case SymTagFunctionArgType:		onFrameVar_UnknownType(c);     break;
			case SymTagFuncDebugStart:		onFrameVar_UnknownType(c);     break;
			case SymTagFuncDebugEnd:		onFrameVar_UnknownType(c);     break;
			case SymTagUsingNamespace:		onFrameVar_UnknownType(c);     break;
			case SymTagVTableShape:			onFrameVar_UnknownType(c);     break;
			case SymTagVTable:				onFrameVar_UnknownType(c);     break;
			case SymTagCustom:				onFrameVar_UnknownType(c);     break;
			case SymTagThunk:				onFrameVar_UnknownType(c);     break;
			case SymTagCustomType:			onFrameVar_UnknownType(c);     break;
			case SymTagManagedType:			onFrameVar_UnknownType(c);     break;
			case SymTagDimension:			onFrameVar_UnknownType(c);     break;
			case SymTagCallSite:			onFrameVar_UnknownType(c);     break;
			default:						onFrameVar_UnknownType(c);     break;
			}
		}

		return (*c.onVariable)(c.userData, &c.var);
	}
}}} // namespace mmk::debug::dbgHelpContext

#endif /* def MMK_DEBUG_STACK_USE_DBGHELP */
