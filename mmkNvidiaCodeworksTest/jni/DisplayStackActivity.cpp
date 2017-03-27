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

#include <mmk/debug/stack.h>

// Private APIs for debugging!
#include "elf.reader.hpp"
#include "dwarf.debugLine.reader.hpp"

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <jni.h>

#define NO_INLINE   ZMMK_DEBUG_STACK_NO_INLINE // Note: ZMMK_* isn't part of the public API




extern "C" JNIEXPORT jstring JNICALL
Java_com_maulingmonkey_debug_stack_nvidiaCodeworksTest_DisplayStackActivity_mmkDebugStackSearchPaths( JNIEnv* env, jobject thiz, jstring jElfDir)
{
	const char* elfDir  = env->GetStringUTFChars(jElfDir,  NULL);
	mmkDebugStackSearchPaths(&elfDir, 1);
	env->ReleaseStringUTFChars(jElfDir,  elfDir);
}




const char* nameOnly(const char* filename) {
	if (const char* bs = strrchr(filename, '\\')) return bs+1;
	if (const char* fs = strrchr(filename, '/'))  return fs+1;
	return filename;
}

std::stringstream lastTrace;
#define TRACE() (void)(lastTrace.str(""),   mmk::debug::stackCurrentThread(mmkDebugStackResolveAll, 0, 25,  \
    [](const mmk::debug::stackFunction& f) {                                                                \
        lastTrace << "    " << nameOnly(f.file) << "(" << f.line << "): " << f.name;                        \
        if (f.functionBase) lastTrace << " + " << (void*)((size_t)f.address - (size_t)f.functionBase);      \
        else                lastTrace << " @ " << f.address;                                                \
        lastTrace << " (" << f.module;                                                                      \
        if (f.moduleBase) lastTrace << " + " << (void*)((size_t)f.address - (size_t)f.moduleBase);          \
        lastTrace << ")\n";                                                                                 \
        return true; /* continue trace */                                                                   \
    }, MMK_DEBUG_STACK_HINT()),   0)

NO_INLINE void test_trace() { TRACE(); }
template < size_t N > struct recursive { NO_INLINE static void test_trace() { recursive<N-1>::test_trace(); } };
template <>        struct recursive<0> { NO_INLINE static void test_trace() { ::test_trace(); } };


extern "C" JNIEXPORT jstring JNICALL
Java_com_maulingmonkey_debug_stack_nvidiaCodeworksTest_DisplayStackActivity_traceDemo( JNIEnv* env, jobject thiz )
{
	recursive<10>::test_trace();
	return env->NewStringUTF(lastTrace.str().c_str());
}




void dumpElf(mmk::debug::elf::reader& p, std::ostream& o)
{
	using namespace mmk::debug;

	elf::fileHeader elf;
	std::vector<char> names;

	if (!p.read(elf)) return;
	if (!p.readSectionNames(elf, names)) return;

	for (size_t i=0; i<elf.sectionHeaderCount; ++i)
	{
		elf::sectionHeader sHeader;
		if (!p.read(elf, i, sHeader)) return;
		const char* sName = p.name(sHeader, names);
		if (!sName) return;

		o << "Section #" << i << ": " << sName << " = " << sHeader.fileOffset << ".." << (sHeader.fileOffset+sHeader.size) << "\n";

		if (strcmp(sName, ".debug_line") == 0) {
			std::vector<char> sBuffer;
			p.read(sHeader, sBuffer);
			dwarf::debugLine::header _prologueBuf;

			if (auto spp = dwarf::debugLine::header::tryParse(sBuffer.data(), sBuffer.size(), _prologueBuf)) {
				const char* end = sBuffer.data() + sBuffer.size();
				if (spp->afterTotal < end) end = spp->afterTotal;

				o << "    totalLen:  " << (unsigned)spp->totalLength << "\n";
				o << "    ver:       " << (unsigned)spp->version << "\n";
				o << "    proLen:    " << (unsigned)spp->prologueLength << "\n";
				o << "    minIns:    " << (unsigned)spp->minimumInstructionLength << "\n";
				o << "    defIsSt:   " << (bool)    spp->defaultIsStatement << "\n";
				o << "    lineBase:  " << (  signed)spp->lineBase << "\n";
				o << "    lineRange: " << (unsigned)spp->lineRange << "\n";
				o << "    specialOp: " << (unsigned)spp->firstSpecialOpcode << "\n";
				o << "    stdops:    " << (unsigned)spp->standardOpcodeLengths.size() << "\n";
				o << "    dirs:      " << (unsigned)spp->dirs.size() << "\n";
				for (const char* dir : spp->dirs) o << "        " << dir << "\n";

				o << "    files:   " << spp->files.size() << "\n";
				for (const auto& file : spp->files) if (file.name) o << "        " << file.name << "\n";

				if (spp->afterPrologue) {
					dwarf::debugLine::reader r(*spp, spp->afterPrologue, end);
					void* prev = NULL;
					r.for_each([&](){
						const char* file = "unknown";
						if (r.file < spp->files.size()) file = spp->files[r.file].name;
						o << "\n";
						if (prev) o << prev << "..";
						o << (void*)r.address << ": " << file << "(" << r.line << ")";
						prev = r.endSequence ? NULL : (void*)r.address;
					},[&](const char* debug){
						o << "  " << debug;
						//o << "op" << opCode << " = " << (void*)r.address << ": " << file << "(" << r.line << ")\n";
					});
					o << "\n";
				}
			}
		}
	}
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_maulingmonkey_debug_stack_nvidiaCodeworksTest_DisplayStackActivity_inspectElf( JNIEnv* env, jobject thiz, jstring jElfDir, jstring jElfName)
{

	const char* elfDir  = env->GetStringUTFChars(jElfDir,  NULL);
	const char* elfName = env->GetStringUTFChars(jElfName, NULL);

	using namespace mmk::debug::elf;
	std::stringstream o;
	o << "ELF: " << elfDir << "/" << elfName << "\n";
	reader p(&elfDir, 1, elfName);
	dumpElf(p, o);
	if (const char* fail = p.failureCause()) { o << "Failure cause: " << fail << "\n"; }

	env->ReleaseStringUTFChars(jElfDir,  elfDir);
	env->ReleaseStringUTFChars(jElfName, elfName);
	return env->NewStringUTF(o.str().c_str());
}



