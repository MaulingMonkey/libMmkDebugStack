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

#ifndef ZMMK_IG_SRC_DWARF_DEBUGLINE_HEADER_HPP
#define ZMMK_IG_SRC_DWARF_DEBUGLINE_HEADER_HPP

// Ref: http://www.dwarfstd.org/doc/dwarf-2.0.0.pdf See 6.2 Line Number Information

#include "dwarf.leb128.hpp"
#include <vector>
#include <stddef.h>

namespace mmk { namespace debug { namespace dwarf { namespace debugLine
{
	// See 6.2.1 Definitions
	typedef   signed char  sbyte;
	typedef unsigned char  ubyte;
	typedef unsigned short uhalf;
	typedef   signed int   sword; // NOT TO BE CONFUSED WITH Win32 "WORD"! DIFFERENT SIZE!
	typedef unsigned int   uword; // NOT TO BE CONFUSED WITH Win32 "WORD"! DIFFERENT SIZE!

	// See 6.2.4 "The Statement Program Prologue"
	struct header {
		struct file {
			const char* name;
			uleb128     directoryIndex;
			uleb128     lastModification;
			uleb128     size;
		};

		uword                    totalLength;
		uhalf                    version;
		uword                    prologueLength;
		ubyte                    minimumInstructionLength;
		ubyte                    defaultIsStatement;
		sbyte                    lineBase;
		ubyte                    lineRange;
		ubyte                    firstSpecialOpcode;

		std::vector<ubyte>       standardOpcodeLengths; // LEB128 s - corresponds to [1..firstSpecialOpcode]
		std::vector<const char*> dirs;
		std::vector<file>        files;

		const char*              afterPrologue;
		const char*              afterTotal;

		template < typename t >
		static bool tryRead(const char*& p, const char* end, t& out) {
			if (sizeof(out) > (size_t)(end-p)) return false;

			out = *(t*)p;
			p += sizeof(out);
			return true;
		}

		static header* tryParse(const char* begin, size_t bufferSize, header& b);
	};
}}}} // namespace mmk::debug::dwarf::debugLine

#endif /* ndef ZMMK_IG_SRC_elf_TYPES_DEBUG_LINE_HPP */
