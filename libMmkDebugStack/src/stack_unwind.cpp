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

#ifndef MMK_DEBUG_STACK_USE_UNWIND
void stack_unwind_exports_nothing() {} // Supress linker warning
#else

#include <mmk/debug/stack.h>
#include "assert.hpp"
#include "elf.reader.hpp"
#include "searchPaths.hpp"
#include "dwarf.debugLine.reader.hpp"

#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>

using namespace mmk::debug;
using namespace mmk::debug::elf;

namespace
{
	struct context
	{
		mmkDebugStackResolveFlags	flags;
		unsigned					framesToSkip;
		unsigned					maxFrames;
		void*						userData;
		int							(*onFunction) (void* userData, const mmkDebugStackFunction*);
		int							(*onVariable) (void* userData, const mmkDebugStackVariable*);
		const char*					fileHint;
		unsigned					lineHint;
		const char*					funcHint;
	};

	bool reportFrame(context& c, uintptr_t ip);

#ifndef __clang__ // _Unwind_Backtrace fails to invoke doUnwind on clang at all, we'll use __builtin_return_address instead.
	_Unwind_Reason_Code doUnwind(_Unwind_Context* uwContext, void* pOurContext)
	{
		if (!ZMMK_ASSERT(pOurContext, "Expected pOurContext")) return _URC_FAILURE;
		//if (!uwContext) return urcEnd; // libunwind bug or terminal condition?
		context& c = *static_cast<context*>(pOurContext);

		if (c.framesToSkip > 0) { --c.framesToSkip; return _URC_NO_REASON; }
		if (c.maxFrames <= 0) { return _URC_FAILURE; } // Terminal completed
		--c.maxFrames;

		const uintptr_t ip = _Unwind_GetIP(uwContext);

		return reportFrame(c, ip) ? _URC_NO_REASON : _URC_FAILURE;
	}
#endif

	bool reportFrame(context& c, uintptr_t ip) {
		if (!ip) return false;
		mmkDebugStackFunction f = {};
		f.functionBase = NULL;
		f.address      = reinterpret_cast<void*>(ip);
		f.file         = (c.flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";
		f.name         = (c.flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";
		f.moduleBase   = NULL;
		f.module       = (c.flags & mmkDebugStackResolveNullMissing) ? NULL : "<unknown>";
		f.line         = 0;

		// Another option:  Parse /proc/self/maps for module info, then parse the elf for naming
		char demangledBuf[1024]="";
		Dl_info dlInfo = {};
		if (c.flags & (mmkDebugStackResolveFuncName | mmkDebugStackResolveFuncModule | mmkDebugStackResolveFuncFile | mmkDebugStackResolveFuncLine)) {
			if (dladdr(f.address, &dlInfo)) {
				if (dlInfo.dli_fname) f.module = dlInfo.dli_fname;
				f.functionBase = dlInfo.dli_saddr;
				if (dlInfo.dli_sname) {
					f.name = dlInfo.dli_sname;
					int status = 0;
					size_t demangledLength = sizeof(demangledBuf)-1;
					if (const char* demangled = __cxxabiv1::__cxa_demangle(dlInfo.dli_sname, demangledBuf, &demangledLength, &status)) f.name = demangled;
				}
				f.moduleBase = dlInfo.dli_fbase;
			}
		}

		fileHeader elf;
		std::string filePath;

		if (dlInfo.dli_fname && (c.flags & (mmkDebugStackResolveFuncFile | mmkDebugStackResolveFuncLine))) {
			std::vector<const char*> search;
			for (size_t i=0; i<stackSearchPaths.size(); ++i) search.push_back(stackSearchPaths[i].c_str());
			reader p(search.data(), search.size(), dlInfo.dli_fname);

			std::vector<char> names;
			if (p.read(elf) && p.readSectionNames(elf, names)) for (size_t i=0; i<elf.sectionHeaderCount; ++i) {
				sectionHeader sHeader;
				if (!p.read(elf, i, sHeader)) continue;

				const char* sName = p.name(sHeader, names);
				if (!sName) continue;

				if (strcmp(sName, ".debug_line") == 0) {
					using namespace mmk::debug::dwarf;

					std::vector<char> sBuffer;
					p.read(sHeader, sBuffer);
					debugLine::header _prologueBuf;

					if (debugLine::header* spp = debugLine::header::tryParse(sBuffer.data(), sBuffer.size(), _prologueBuf)) {
						const char* end = sBuffer.data() + sBuffer.size();
						if (spp->afterTotal < end) end = spp->afterTotal;
						if (!spp->afterPrologue) continue;

						size_t relAddress = (size_t)((char*)f.address - (char*)f.moduleBase);

						debugLine::reader r(*spp, spp->afterPrologue, end);

						size_t prevAddress = 0;
						size_t prevFile = 0;
						size_t prevLine = 0;
						r.for_each([&](){
							if (prevAddress && prevAddress <= relAddress && relAddress < r.address) {
								// Match!
								if (prevFile >= spp->files.size()) {
									filePath = "unknown";
								} else {
									const auto& fileEntry = spp->files[prevFile];

									const char* const dir
										= fileEntry.directoryIndex == 0               ? "."
										: fileEntry.directoryIndex < spp->dirs.size() ? spp->dirs[fileEntry.directoryIndex]
										: "<invalid index>";

									filePath = dir;
									filePath += "/";
									filePath += fileEntry.name;
								}
								f.file = filePath.c_str();
								f.line = prevLine;
							}
							prevAddress = r.address;
							prevFile    = r.file;
							prevLine    = r.line;
							if (r.endSequence) prevAddress = 0;
						});
					}
				}
			}

			if (const char* failure = p.failureCause()) {
				filePath = "FAIL: ";
				filePath += failure;
				f.module = filePath.c_str();
			}
		}

		return (*c.onFunction)(c.userData, &f);
	}

}

ZMMK_DEBUG_STACK_API
ZMMK_DEBUG_STACK_NO_INLINE
void mmkDebugStackCurrentThread(
	mmkDebugStackResolveFlags	flags,
	unsigned					framesToSkip,
	unsigned					maxFrames,
	void*						userData,
	int							(*onFunction) (void* userData, const mmkDebugStackFunction*),
	int							(*onVariable) (void* userData, const mmkDebugStackVariable*),
	const char*					fileHint,
	unsigned					lineHint,
	const char*					funcHint)
{
	// Preconditions

	// framesToSkip may be 0
	if (!ZMMK_ASSERT(maxFrames > 0, "Expected maxFrames > 0")) return;
	// flags may be 0
	// userData may be NULL
	if (!ZMMK_ASSERT(onFunction, "Expected onFunction callback")) return;
	// onVariable may be NULL
	// fileHint may be NULL
	// lineHint may be 0
	// funcHint may be NULL



	// framesToSkip += 1; // Skip self
	MMK_DEBUG_STACK_LOCK_SCOPE();
	zmmkDebugStackInitNoLock();

	context ourContext = { flags, framesToSkip, maxFrames, userData, onFunction, onVariable, fileHint, lineHint, funcHint };

#ifdef __clang__ // _Unwind_Backtrace fails to invoke doUnwind on clang at all, so use __builtin_return_address instead.
#define FRAME(n) if (n >= framesToSkip) {                                                                        \
    if (maxFrames-- == 0) return;                                                                                \
    if (!reportFrame(ourContext, (uintptr_t)__builtin_extract_return_addr(__builtin_return_address(n)))) return; \
}
#define FRAME10(n) FRAME(n##0) FRAME(n##1) FRAME(n##2) FRAME(n##3) FRAME(n##4) FRAME(n##5) FRAME(n##6) FRAME(n##7) FRAME(n##8) FRAME(n##9)
	// Up to 100 frames
	FRAME10() FRAME10(1) FRAME10(2) FRAME10(3) FRAME10(4) FRAME10(5) FRAME10(6) FRAME10(7) FRAME10(8) FRAME10(9)
#undef FRAME10
#undef FRAME

#else // While we can call __builtin_frame_address on GCC, it crashes!  So let's not do that.
	_Unwind_Reason_Code rc = _Unwind_Backtrace(&doUnwind, &ourContext);
	(void)rc;
#endif
}

#endif /* def MMK_DEBUG_STACK_USE_DBGHELP */
