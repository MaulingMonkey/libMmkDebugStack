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

#ifndef ZMMK_IG_SRC_DWARF_DEBUGLINE_OPCODES_HPP
#define ZMMK_IG_SRC_DWARF_DEBUGLINE_OPCODES_HPP

// Ref: http://www.dwarfstd.org/doc/dwarf-2.0.0.pdf

namespace mmk { namespace debug { namespace dwarf { namespace debugLine {
	// See 6.2.5.2 Standard Opcodes
	enum stdOpCode {
		// DWARF 2
		stdOpCopy         = 1,    // ()                  append row; basicBlock = false
		stdOpAdvancePc,           // (uleb128 v)         address     += v * minimumInstructionLength
		stdOpAdvanceLine,         // (sleb128 dLine)     line        += dLine
		stdOpSetFile,             // (uleb128 newFile)   file         = newFile
		stdOpSetColumn,           // (uleb128 newColumn) column       = newColumn
		stdOpToggleIsStatement,   // ()                  isStatement ^= true
		stdOpSetBasicBlock,       // ()                  basicBlock   = true
		stdOpConstAddPc,          // ()                  advance     += ...????!?!?
		stdOpFixedAdvancePc,      // (uhalf v)           address     += v

		// DWARF ?
		stdOpSetPrologueEnd,
		stdOpSetEpilogueBegin,
		stdOpSetIsa
	};

	// See 6.2.5.3 Extended Opcodes
	enum extOpCode {
		// DWARF 2
		extOpEndSequence = 1, // (?)                    Emit last row, reset VM
		extOpSetAddress,      // (void* relocAddress)
		extOpDefineFile       // (const char* srcFile, uleb128 directoryIndex, uleb128 lastMod, uleb128 bytes)
		// Encountering extended opcodes undocumented in DWARF 5
	};
}}}} // namespace mmk::debug::dwarf::debugLine

#endif /* ndef ZMMK_IG_SRC_DWARF_DEBUGLINE_OPCODES_HPP */
