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

#include "dwarf.debugLine.header.hpp"
#include "dwarf.leb128.hpp"

namespace mmk { namespace debug { namespace dwarf { namespace debugLine {
	header* header::tryParse(const char* begin, size_t bufferSize, header& b) {
		const char* p = begin;
		const char* const end = begin + bufferSize;

		if (!tryRead(p, end, b.totalLength				)) return NULL;
		if (b.totalLength > bufferSize					)  return NULL;
		if (!tryRead(p, end, b.version					)) return NULL;
		if (!tryRead(p, end, b.prologueLength			)) return NULL;
		if (!tryRead(p, end, b.minimumInstructionLength	)) return NULL;
		if (!tryRead(p, end, b.defaultIsStatement		)) return NULL;
		if (!tryRead(p, end, b.lineBase					)) return NULL;
		if (!tryRead(p, end, b.lineRange				)) return NULL;
		if (!tryRead(p, end, b.firstSpecialOpcode		)) return NULL;
		if (b.firstSpecialOpcode == 0					) return NULL; // 0 is reserved
		b.standardOpcodeLengths.reserve(b.firstSpecialOpcode-1); // don't include index 0

		for (ubyte standardOpCode = 1; standardOpCode < b.firstSpecialOpcode; ++standardOpCode) {
			ubyte length = 0;
			if (!tryRead(p, end, length)) return NULL;
			b.standardOpcodeLengths.push_back(length);
		}

		while (p < end && *p) {
			const char* name = p;
			while (*++p) if (p >= end) return NULL;
			b.dirs.push_back(name);
			++p;
		}
		++p; // Skip terminal nul

		b.files.push_back(file()); // index 0 is skipped
		while (p < end && *p) {
			file f = {};
			f.name = p;
			while (*++p) if (p >= end) return NULL;
			++p;
			if (!tryReadLeb128(p, end, f.directoryIndex		)) return NULL;
			if (!tryReadLeb128(p, end, f.lastModification	)) return NULL;
			if (!tryReadLeb128(p, end, f.size				)) return NULL;
			b.files.push_back(f);
		}
		++p; // Skip terminal nul

		b.afterTotal    = begin + sizeof(b.totalLength)                                                + b.totalLength;
		b.afterPrologue = begin + sizeof(b.totalLength) + sizeof(b.version) + sizeof(b.prologueLength) + b.prologueLength;
		if (p > b.afterTotal   ) b.afterTotal    = NULL; // Error
		if (p > b.afterPrologue) b.afterPrologue = NULL; // Error

		return &b;
	}
}}}} // namespace mmk::debug::dwarf::debugLine
