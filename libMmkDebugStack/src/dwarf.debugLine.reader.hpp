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

#ifndef ZMMK_IG_SRC_DWARF_DEBUGLINE_READER_HPP
#define ZMMK_IG_SRC_DWARF_DEBUGLINE_READER_HPP

// http://www.dwarfstd.org/doc/dwarf-2.0.0.pdf See 6.2 Line Number Information

#include "dwarf.debugLine.header.hpp"
#include "dwarf.debugLine.opcodes.hpp"
#include <vector>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

namespace mmk { namespace debug { namespace dwarf { namespace debugLine {
	struct reader {
		const header& spp;

		// See 6.2.2 State Machine Registers
		size_t address;
		uword  file;
		uword  line;
		uword  column;
		bool   isStatement;
		bool   basicBlock;
		bool   endSequence;

		const char* const begin;
		const char* const end;
		const char*       next;

		explicit reader(const header& spp, const char* begin, const char* end)
			: spp(spp)

			, address(0)
			, file(1)
			, line(1)
			, column(0)
			, isStatement(!!spp.defaultIsStatement)
			, basicBlock(false)
			, endSequence(false)

			, begin(begin)
			, end(end)
			, next(begin)
		{
		}

		size_t remaining() const { return (size_t)(end - next); }

		ubyte readOpCode() {
			if (next < end) return *next++;
			return 0;
		}

		uhalf readHalf() {
			if (next + sizeof(uhalf) > end) return 0;
			uhalf v=0;
			memcpy(&v, next, sizeof(v));
			next += sizeof(v);
			return v;
		}

		struct ignoreDebug { void operator()(const char*) const {} };

		template < typename Emit > void for_each(const Emit& emit) { for_each(emit, ignoreDebug()); }

		template < typename Emit, typename Debug > void for_each(const Emit& emit, const Debug& debug) {
			using namespace mmk::debug::dwarf; // temporary
			using namespace mmk::debug::dwarf::debugLine; // temporary
#if 0 // Debug info
			char debugBuf[1024]="";
			#define DEBUG_FMT(...) snprintf(debugBuf, sizeof(debugBuf), __VA_ARGS__); debug(debugBuf)
#else
			#define DEBUG_FMT(...)
#endif

			for (next = begin; next<end;) {
				const unsigned opCode = readOpCode();
				//DEBUG_FMT("op: %d", opCode);

				if (opCode == 0) { // Extended opcode
					uleb128 argsBytes = 0; // Includes the subOpCode but not the length itself nor the opCode
					if (!tryReadLeb128(next, end, argsBytes)) break;
					if (next >= end) break;
					const char subOpCode = *next++;
					//DEBUG_FMT("subop: %d", subOpCode);

					switch (subOpCode)
					{
					case extOpEndSequence: DEBUG_FMT("extOpEndSequence (len: %d)", argsBytes);
						endSequence = true;
						emit();
						address     = 0;
						file        = 1;
						line        = 1;
						column      = 0;
						isStatement = spp.defaultIsStatement;
						basicBlock  = false;
						endSequence = false;
						break;
					case extOpSetAddress:
						if (remaining() < sizeof(void*)) return; // Bug
						address = *(size_t*)next;
						DEBUG_FMT("extOpSetAddress: %p (len: %d)", (void*)address, argsBytes);
						next += sizeof(size_t);
						break;
					case extOpDefineFile:
						// TODO
						DEBUG_FMT("extOpDefineFile (len: %d)", argsBytes);
						next += argsBytes-1;
						break;
					default: DEBUG_FMT("extOp?%d? (len: %d)", subOpCode, argsBytes);
						next += argsBytes-1;
						break;
					}
				} else if (opCode < spp.firstSpecialOpcode) { // Standard opcode
					const unsigned len = spp.standardOpcodeLengths[opCode-1]; // 1st standard opcode is #1

					switch (opCode)
					{
					case stdOpCopy:              { emit(); basicBlock = false;                            DEBUG_FMT("stdOpCopy"             );                                              } break;
					case stdOpAdvancePc:         { uleb128 v=0; if (!tryReadLeb128(next, end, v)) return; DEBUG_FMT("stdOpAdvancePc %d",   v); address += v * spp.minimumInstructionLength; } break;
					case stdOpAdvanceLine:       { sleb128 v=0; if (!tryReadLeb128(next, end, v)) return; DEBUG_FMT("stdOpAdvanceLine %i", v); line    += v;                                } break;
					case stdOpSetFile:           { uleb128 v=0; if (!tryReadLeb128(next, end, v)) return; DEBUG_FMT("stdOpSetFile %d",     v); file     = v;                                } break;
					case stdOpSetColumn:         { uleb128 v=0; if (!tryReadLeb128(next, end, v)) return; DEBUG_FMT("stdOpSetColumn %d",   v); column   = v;                                } break;
					case stdOpToggleIsStatement: { isStatement ^= true;                                   DEBUG_FMT("stdOpToggleIsStatement");                } break;
					case stdOpSetBasicBlock:     { basicBlock   = true;                                   DEBUG_FMT("stdOpSetBasicBlock"    );                } break;
					case stdOpConstAddPc:        { address += spp.minimumInstructionLength * ((255 - spp.firstSpecialOpcode) / spp.lineRange); DEBUG_FMT("stdOpConstAddPc"); } break;
					case stdOpFixedAdvancePc:    { uhalf v=readHalf(); address += v;                      DEBUG_FMT("stdOpFixedAdvancePc %d", v);             } break;
					default: DEBUG_FMT("stdOp?%d?", opCode);
						for (unsigned i=0; i<len; ++i) {
							uleb128 arg = 0;
							if (!tryReadLeb128(next, end, arg)) break;
						}
					}

				} else { // Special opcode
					const unsigned noSpecial = opCode - spp.firstSpecialOpcode;
					const   signed dLine     = spp.lineBase + (noSpecial % spp.lineRange);
					const unsigned dAddress  = noSpecial / spp.lineRange;
					DEBUG_FMT("spc%d: %i %i", opCode, dLine, dAddress);

					line    += dLine;
					address += dAddress * spp.minimumInstructionLength;
					emit();
					basicBlock = false;
				}
			}
		}
	};
}}}} // namespace mmk::debug::dwarf::debugLine

#endif /* ndef ZMMK_IG_SRC_DWARF_DEBUGLINE_READER_HPP */
