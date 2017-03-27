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

#ifndef ZMMK_IG_SRC_DWARF_LEB128_HPP
#define ZMMK_IG_SRC_DWARF_LEB128_HPP

// Ref file:///I:/home/media/documentation/Dwarf/dwarf-2.0.0.pdf

namespace mmk { namespace debug { namespace dwarf
{
	// See 7.6 "Variable Length Data."  Note that LEB128 = "Little Endian Base 128" (basic RLE encoding scheme w/ high bit = continue)

	typedef unsigned int uleb128;
	typedef   signed int sleb128;

	bool tryReadLeb128(const char*& p, const char* end, uleb128& out);
	bool tryReadLeb128(const char*& p, const char* end, sleb128& out);
}}} // namespace mmk::debug::dwarf

#endif /* ndef ZMMK_IG_SRC_DWARF_LEB128_HPP */
