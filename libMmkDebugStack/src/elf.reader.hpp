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

#ifndef ZMMK_IG_SRC_ELF_READER_HPP
#define ZMMK_IG_SRC_ELF_READER_HPP

#include "elf.types.hpp"
#include <vector>
#include <stddef.h>
#include <stdio.h>

namespace mmk { namespace debug { namespace elf {
	class reader {
		char failureCauseBuffer[1024];
		FILE* f;
		typedef size_t fileoff; // >4GB ELF? Surely not...

		FILE* fopenSearch(const char** searchPaths, size_t searchPathsCount, const char* name);
		FILE* fopenSingle(const char* path);
	public:
		explicit reader(const char** searchPaths, size_t searchPathsCount, const char* name);
		explicit reader(const char* path);
		~reader();

		const char* failureCause() const;

		bool read(fileHeader& elf);
		bool read(const fileHeader& elf, size_t index, sectionHeader& header);
		bool read(const fileHeader& elf, size_t index, programHeader& header);
		bool read(const sectionHeader& header, std::vector<char>& buf);
		bool read(const fileHeader& elf, size_t index, sectionHeader& header, std::vector<char>& buf);
		bool readSectionNames(const fileHeader& elf, std::vector<char>& namesBuf);
		const char* name(const sectionHeader& header, const std::vector<char>& namesBuf);
		bool seek(fileoff o);
		bool read(void* buffer, fileoff n);
	private:
		void report_error(const char* condition, const char* file, size_t line);

		reader(const reader&); // uncopyable
		reader& operator=(const reader&); // unassignable
	};
}}} // namespace mmk::debug::elf

#endif /* ndef ZMMK_IG_SRC_ELF_READER_HPP */
