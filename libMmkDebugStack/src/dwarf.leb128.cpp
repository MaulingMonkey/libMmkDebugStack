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

#include "dwarf.leb128.hpp"

namespace mmk { namespace debug { namespace dwarf
{
	bool tryReadLeb128(const char*& p, const char* end, uleb128& out) {
		unsigned shift = 0;
		uleb128 value = 0;

		while (p < end) {
			unsigned char b = *p++;
			unsigned char v = b & 0x7f;
			value |= v << shift;
			shift += 7;

			if (b == v) {
				out = value;
				return true; // end of LEB
			}
		}
		return false; // end of buffer!
	}

	bool tryReadLeb128(const char*& p, const char* end, sleb128& out) {
		unsigned shift = 0;
		uleb128 value = 0;

		while (p < end) {
			unsigned char b = *p++;
			unsigned char v = b & 0x7f;
			value |= v << shift;
			shift += 7;

			if (b == v) {
				const bool signBit = !!((1u << (shift-1)) & value);
				if (signBit) value |= ((uleb128)-1) << shift; // sign extend
				out = (sleb128)value;
				return true; // end of LEB
			}
		}
		return false; // end of buffer!
	}
}}} // namespace mmk::debug::dwarf
