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

#include "elf.reader.hpp"
#include "assert.hpp"

#include <algorithm>

#include <memory.h>
#include <errno.h>
#include <limits.h>

//    Sources:
// https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
// http://www.skyfree.org/linux/references/ELF_Format.pdf


namespace mmk { namespace debug { namespace elf {

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4996) // fopen is unsafe? really?
#endif

	FILE* reader::fopenSearch(const char** searchPaths, size_t searchPathsCount, const char* name) {
		failureCauseBuffer[0] = '\0';
		if (searchPathsCount == 0) {
			snprintf(failureCauseBuffer, sizeof(failureCauseBuffer), "fopenSearch(..., \"%s\") failed:  No search paths!", name);
			return NULL;
		}
		for (size_t i=0; i<searchPathsCount; ++i) {
			char path[1024] = "";
			snprintf(path, sizeof(path), "%s/%s", searchPaths[i], name);
			if (FILE* file = fopen(path, "r")) return file;
		}
		snprintf(failureCauseBuffer, sizeof(failureCauseBuffer), "fopenSearch(..., \"%s\") failed to open any file!", name);
		return NULL;
	}

	FILE* reader::fopenSingle(const char* path) {
		failureCauseBuffer[0] = '\0';
		if (FILE* file = fopen(path, "r")) return file;

		snprintf(failureCauseBuffer, sizeof(failureCauseBuffer), "fopen(\"%s\", \"r\") failed with errno=%d", path, errno);
		return NULL;
	}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

	reader::reader(const char** searchPaths, size_t searchPathsCount, const char* name)
		: failureCauseBuffer()
		, f(fopenSearch(searchPaths, searchPathsCount, name))
	{
	}

	reader::reader(const char* path)
		: failureCauseBuffer()
		, f(fopenSingle(path))
	{
	}

	reader::~reader() {
		if (f) fclose(f);
	}

	const char* reader::failureCause() const { return (failureCauseBuffer[0] != '\0') ? failureCauseBuffer : NULL; }

#define PARSE_EXPECT(condition) if (!(condition)) { report_error("!(" #condition ")", __FILE__, __LINE__); return 0; } // May be false, may be NULL

	bool reader::read(fileHeader& elf) {
		PARSE_EXPECT(seek(0));
		PARSE_EXPECT(read(&elf, sizeof(elf)));
		PARSE_EXPECT(memcmp(elf.ident.magic, "\x7F""ELF", 4) == 0);
		PARSE_EXPECT(elf.ident.bitness == fileBitness32 || elf.ident.bitness == fileBitness64);
		PARSE_EXPECT(elf.ident.endian == fileEndianLittle || elf.ident.endian == fileEndianBig);
		PARSE_EXPECT(elf.ident.version == 1);
		PARSE_EXPECT(elf.type == fileTypeRelocatable || elf.type == fileTypeExecutable || elf.type == fileTypeShared || elf.type == fileTypeCore);
		// ...
		PARSE_EXPECT(elf.size == sizeof(elf));
		// ...
		PARSE_EXPECT(elf.sectionHeaderCount < 1000);
		PARSE_EXPECT(elf.programHeaderCount < 1000);
		PARSE_EXPECT(elf.sectionHeaderNamesIndex < elf.sectionHeaderCount);
		return true;
	}

	bool reader::read(const fileHeader& elf, size_t index, sectionHeader& header) {
		PARSE_EXPECT(index < elf.sectionHeaderCount);
		sectionHeader h = {};
		PARSE_EXPECT(seek(elf.sectionHeaderTable + index * elf.sectionHeaderEntrySize));
		PARSE_EXPECT(read(&h, std::min((size_t)elf.sectionHeaderEntrySize, sizeof(h))));
		header = h;
		return true;
	}

	bool reader::read(const fileHeader& elf, size_t index, programHeader& header) {
		PARSE_EXPECT(index < elf.programHeaderCount);
		programHeader h = {};
		PARSE_EXPECT(seek(elf.programHeaderTable + index * elf.programHeaderEntrySize));
		PARSE_EXPECT(read(&h, std::min((size_t)elf.programHeaderEntrySize, sizeof(h))));
		header = h;
		return true;
	}

	bool reader::read(const sectionHeader& header, std::vector<char>& buf) {
		PARSE_EXPECT(header.size <= 10000000); // 10MB
		buf.resize(header.size);
		PARSE_EXPECT(seek(header.fileOffset));
		PARSE_EXPECT(read(buf.data(), buf.size()));
		return true;
	}

	bool reader::read(const fileHeader& elf, size_t index, sectionHeader& header, std::vector<char>& buf) {
		return read(elf, index, header) && read(header, buf);
	}

	bool reader::readSectionNames(const fileHeader& elf, std::vector<char>& namesBuf) {
		sectionHeader namesHeader;
		if (!read(elf, elf.sectionHeaderNamesIndex, namesHeader, namesBuf)) return false;
		if (namesBuf.back() == '\0') return true;
		namesBuf.push_back('\0'); // Safety measure: ensure NUL terminated
		PARSE_EXPECT(!"Section names buffer wasn't NUL terminated");
		return false;
	}

	const char* reader::name(const sectionHeader& header, const std::vector<char>& namesBuf) {
		PARSE_EXPECT(header.nameShstrtabOffset < namesBuf.size());
		return namesBuf.data() + header.nameShstrtabOffset;
	}

	bool reader::seek(fileoff o) {
		if (!f) return false;
		PARSE_EXPECT(o <= LONG_MAX);
		return fseek(f, (long)o, SEEK_SET)==0;
	}

#undef PARSE_EXPECT

	bool reader::read(void* buffer, fileoff n) {
		if (!f) return false;
		return fread(buffer, n, 1, f) == 1;
	}

	void reader::report_error(const char* condition, const char* file, size_t line) {
		if (!f) return; // already failed
		if (f) fclose(f);
		f = NULL;
		//fprintf(stderr, "%s(%d): Parse failed: %s\n", file, line, condition);
		snprintf(failureCauseBuffer, sizeof(failureCauseBuffer), "%s(%d): Parse failed: %s\n", file, (unsigned)line, condition);
	}
}}} // namespace mmk::debug::elf
