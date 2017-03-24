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
void dbgHelpContext_baseType_exports_nothing() {} // Supress linker warning
#else
#include "dbgHelpContext.hpp"
#include "assert.hpp"

namespace mmk { namespace debug { namespace dbgHelpContext
{
#ifdef _IMAGEHLP64
	DWORD64 GetRegValue(CONTEXT& c, DWORD reg)
#else
	DWORD GetRegValue(CONTEXT& c, DWORD reg)
#endif
	{
		typedef unsigned char		u8;
		typedef unsigned short		u16;
		typedef unsigned int		u32;
#ifdef _IMAGEHLP64
		typedef unsigned long long	u64;
#else
#define Rax Eax
#define Rbx Ebx
#define Rcx Ecx
#define Rdx Edx
#define Rsp Esp
#define Rbp Ebp
#define Rsi Esi
#define Rdi Edi
#define Rip Eip
#endif

		switch (reg)
		{
		// https://github.com/Microsoft/microsoft-pdb/blob/master/include/cvconst.h#L3285
		// https://msdn.microsoft.com/en-au/library/a0fcdkb9(VS.71).aspx
		case   1: return (u8)c.Rax; // AL
		case   2: return (u8)c.Rcx; // CL
		case   3: return (u8)c.Rdx; // DL
		case   4: return (u8)c.Rbx; // BL
		case   5: return (u8)(c.Rax >> 8); // AH
		case   6: return (u8)(c.Rcx >> 8); // CH
		case   7: return (u8)(c.Rdx >> 8); // DH
		case   8: return (u8)(c.Rbx >> 8); // BH
		case   9: return (u16)c.Rax; // AX
		case  10: return (u16)c.Rcx; // CX
		case  11: return (u16)c.Rdx; // DX
		case  12: return (u16)c.Rbx; // BX
		case  13: return (u16)c.Rsp; // SP
		case  14: return (u16)c.Rbp; // BP
		case  15: return (u16)c.Rsi; // SI
		case  16: return (u16)c.Rdi; // DI
		case  17: return (u32)c.Rax; // EAX
		case  18: return (u32)c.Rcx; // ECX
		case  19: return (u32)c.Rdx; // EDX
		case  20: return (u32)c.Rbx; // EBX
		case  21: return (u32)c.Rsp; // ESP
		case  22: return (u32)c.Rbp; // EBP
		case  23: return (u32)c.Rsi; // ESI
		case  24: return (u32)c.Rdi; // EDI
		case  25: return c.SegEs;
		case  26: return c.SegCs;
		case  27: return c.SegSs;
		case  28: return c.SegDs;
		case  29: return c.SegFs;
		case  30: return c.SegGs;
		// 31
		case  32: return c.ContextFlags;
		case  33: return c.Rip;
		case  34: return c.EFlags;

		// 80-88 = CR 80-88

		case  90: return c.Dr0;
		case  91: return c.Dr1;
		case  92: return c.Dr2;
		case  93: return c.Dr3;
		//case  94: return c.Dr4;
		//case  95: return c.Dr5;
		case  96: return c.Dr6;
		case  97: return c.Dr7;
		//case  98: return c.Dr8;
		//case  99: return c.Dr9;
		//case 100: return c.Dr10;
		//case 101: return c.Dr11;
		//case 102: return c.Dr12;
		//case 103: return c.Dr13;
		//case 104: return c.Dr14;
		//case 105: return c.Dr15;

#ifdef _IMAGEHLP64
		case 328: return c.Rax;
		case 329: return c.Rbx;
		case 330: return c.Rcx;
		case 331: return c.Rdx;
		case 332: return c.Rsi;
		case 333: return c.Rdi;
		case 334: return c.Rbp;
		case 335: return c.Rsp;

		case 336: return c.R8;
		case 337: return c.R9;
		case 338: return c.R10;
		case 339: return c.R11;
		case 340: return c.R12;
		case 341: return c.R13;
		case 342: return c.R14;
		case 343: return c.R15;

		case 344: return (u8)c.R8;
		case 345: return (u8)c.R9;
		case 346: return (u8)c.R10;
		case 347: return (u8)c.R11;
		case 348: return (u8)c.R12;
		case 349: return (u8)c.R13;
		case 350: return (u8)c.R14;
		case 351: return (u8)c.R15;

		case 352: return (u16)c.R8;
		case 353: return (u16)c.R9;
		case 354: return (u16)c.R10;
		case 355: return (u16)c.R11;
		case 356: return (u16)c.R12;
		case 357: return (u16)c.R13;
		case 358: return (u16)c.R14;
		case 359: return (u16)c.R15;

		case 360: return (u32)c.R8;
		case 361: return (u32)c.R9;
		case 362: return (u32)c.R10;
		case 363: return (u32)c.R11;
		case 364: return (u32)c.R12;
		case 365: return (u32)c.R13;
		case 366: return (u32)c.R14;
		case 367: return (u32)c.R15;
#endif
		}

#ifndef _IMAGEHLP64
#undef Rax
#undef Rbx
#undef Rcx
#undef Rdx
#undef Rsp
#undef Rbp
#undef Rsi
#undef Rdi
#undef Rip
#endif

		return 0;
	}

	DWORD64 GetRegValue(dhContext& c)
	{
		if (c.varDhSymbol->Flags & SYMFLAG_VALUEPRESENT)
		{
			return c.varDhSymbol->Value;
		}
		else if (c.varDhSymbol->Flags & SYMFLAG_REGREL)
		{
			switch (c.varSizeof)
			{
			case 1: return *(unsigned char*)     (c.var.address = (void*)(GetRegValue(c.functionContext, c.varDhSymbol->Register) + c.varDhSymbol->Address));
			case 2: return *(unsigned short*)    (c.var.address = (void*)(GetRegValue(c.functionContext, c.varDhSymbol->Register) + c.varDhSymbol->Address));
			case 4: return *(unsigned int*)      (c.var.address = (void*)(GetRegValue(c.functionContext, c.varDhSymbol->Register) + c.varDhSymbol->Address));
			case 8: return *(unsigned long long*)(c.var.address = (void*)(GetRegValue(c.functionContext, c.varDhSymbol->Register) + c.varDhSymbol->Address));
			}
			
		}
		else if (c.varDhSymbol->Flags & SYMFLAG_REGISTER)
		{
			return GetRegValue(c.functionContext, c.varDhSymbol->Register);
		}
		else
		{
			// ...?
		}

		return 0;
	}


	const char* pow2names(DWORD size, const char* unknown, const char* s1, const char* s2, const char* s4, const char* s8, const char* s16) {
		switch (size) {
		case 1:		return s1;
		case 2:		return s2;
		case 4:		return s4;
		case 8:		return s8;
		case 16:	return s16;
		default:	return unknown;
		}
	}

	const char* baseTypeName(DWORD type, DWORD size)
	{
		// https://github.com/TrinityCore/TrinityCore/blob/master/src/common/Debugging/WheatyExceptionReport.h#L58

		switch (type) {
		case 0:			return "None";
		case 1:			return "void";
		case 2:			return "char";
		case 3:			return "wchar_t";
			// 4..5
		case 6:			return pow2names(size,           "int??",          "char",          "short",           "int",          "long long",          "__int128"); // signed integer
		case 7:			return pow2names(size,  "unsigned int??", "unsigned char", "unsigned short",  "unsigned int", "unsigned long long", "unsigned __int128"); // unsigned integer
		case 13:		return pow2names(size,          "long??",          "char",          "short",          "long",          "long long",          "__int128"); // signed long
		case 14:		return pow2names(size, "unsigned long??", "unsigned char", "unsigned short", "unsigned long", "unsigned long long", "unsigned __int128"); // unsigned long

		case 8: // floating point
			switch (size)
			{
			case 2:		return "half";
			case 4:		return "float";
			case 8:		return "double";
			case 12:	return "long double";
			default:	return "float??";
			}

		case 9:			return "BCD"; // Binary-coded decimal type
		case 10:		return "bool";
		// 11..12 (short / unsigned short?)

		// 15..24 ( int8...128, uint8...128 ?)
		case 25:	return "currency";
		case 26:	return "DATE";
		case 27:	return "VARIANT";
		case 28:	return "complex";
		case 29:	return "bit";
		case 30:	return "BSTR";
		case 31:	return "HRESULT";
		case 32:	return "char16_t";
		case 33:	return "char32_t";
		default:	return NULL;
		}
	}

	template < size_t N >
	void baseTypeValueFormat(char (&buffer)[N], DWORD type, dhContext& c)
	{
		// https://github.com/TrinityCore/TrinityCore/blob/master/src/common/Debugging/WheatyExceptionReport.h#L58

		switch (type) {
		case 0:			snprintf(buffer, sizeof(buffer), ""); return;
		case 1:			snprintf(buffer, sizeof(buffer), ""); return;
		case 2:			snprintf(buffer, sizeof(buffer), "'%c'",  (char)         GetRegValue(c)); return;
		case 3:			snprintf(buffer, sizeof(buffer), "L'%c'", (char)(wchar_t)GetRegValue(c)); return;
			// 4..5
		case 6:
		case 13:
			switch (c.varSizeof)
			{
			case 1:		snprintf(buffer, sizeof(buffer), "%d",   (signed char     )(unsigned char     )GetRegValue(c)); return;
			case 2:		snprintf(buffer, sizeof(buffer), "%d",   (signed short    )(unsigned short    )GetRegValue(c)); return;
			case 4:		snprintf(buffer, sizeof(buffer), "%d",   (signed int      )(unsigned int      )GetRegValue(c)); return;
			case 8:		snprintf(buffer, sizeof(buffer), "%lld", (signed long long)(unsigned long long)GetRegValue(c)); return;
			}
		case 7:
		case 14:
			switch (c.varSizeof)
			{
			case 1:		snprintf(buffer, sizeof(buffer), "%u",   (unsigned char     )GetRegValue(c)); return;
			case 2:		snprintf(buffer, sizeof(buffer), "%u",   (unsigned short    )GetRegValue(c)); return;
			case 4:		snprintf(buffer, sizeof(buffer), "%u",   (unsigned int      )GetRegValue(c)); return;
			case 8:		snprintf(buffer, sizeof(buffer), "%llu", (unsigned long long)GetRegValue(c)); return;
			}

		case 8: // floating point
			//switch (c.varSizeof)
			//{
			//case 4:		snprintf(buffer, sizeof(buffer), "%d", (unsigned char     )GetRegValue(c)); return;
			//case 8:		snprintf(buffer, sizeof(buffer), "%d", (unsigned short    )GetRegValue(c)); return;
			//case 12:		snprintf(buffer, sizeof(buffer), "%d", (unsigned int      )GetRegValue(c)); return;
			//}

		case 9:			return; // Binary-coded decimal type
		case 10:		snprintf(buffer, sizeof(buffer), "%s", GetRegValue(c) ? "true" : "false"); return;
		// 11..12 (short / unsigned short?)

		// 15..24 ( int8...128, uint8...128 ?)
		case 25:	return; // "currency";
		case 26:	return; // "DATE";
		case 27:	return; // "VARIANT";
		case 28:	return; // "complex";
		case 29:	return; // "bit";
		case 30:	return; // "BSTR";
		case 31:	return; // "HRESULT";
		case 32:	return; // "char16_t";
		case 33:	return; // "char32_t";
		default:	return; // NULL;
		}
	}

	bool onFrameVar_BaseType(dhContext& c) {
		DWORD baseType = 0;
		if (ZMMK_ASSERT(SymGetTypeInfo(c.process, c.varDhSymbol->ModBase, c.varDhSymbol->TypeIndex, TI_GET_BASETYPE, &baseType), "SymGetTypeInfo(...) failed"))
		{
			if (const char* const type = baseTypeName(baseType, c.varSizeof)) c.var.type = type;

			char buf[1024] = "";
			baseTypeValueFormat(buf, baseType, c);
			//const DWORD64 value = GetRegValue(c);
			//snprintf(buf, sizeof(buf), "%p", (void*)value);
			c.var.value = buf;
		}
		return !!(*c.onVariable)(c.userData, &c.var);
	}
}}} // namespace mmk::debug::dbgHelpContext

#endif /* def MMK_DEBUG_STACK_USE_DBGHELP */
