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
void modules_procmap_exports_nothing() {} // Supress linker warning
#else

#include <mmk/debug/modules.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "assert.hpp"

void mmkDebugModulesLoaded(
	mmkDebugModulesResolveFlags        flags,
	void*                              userData,
	int                                (*onModule)(void* userData,const mmkDebugModule*))
{
	FILE* f = fopen("/proc/self/maps", "r");
	if (!ZMMK_ASSERT(f, "Failed to open /proc/self/maps: errno=%d", errno)) return;

	void*           low             =0;
	void*           hi              =0;
	char            perms[128]      ="";
	unsigned int    offset          =0;
	char            time[128]       ="";
	unsigned int    size            =0;
	char            whitespace[256] ="";
	char            path[256]       ="";
	char            prevPath[256]   ="";

	const bool useNull = flags & mmkDebugModulesResolveNullMissing;
	for (;;) {
		const int components = fscanf(f, "%p-%p %127s %x %127s %d%255[ ]%255[^\n]", &low, &hi, perms, &offset, time, &size, whitespace, path);
		if (components <= 0) break; // Bug or EOF.
		if (!ZMMK_ASSERT(components == 7 || components == 8, "fscanf failed to read a full /proc/self/maps line")) return;

		if (path[0] == '\0') continue; // Skip memory regions that didn't map to anything
		if (path[0] == '[') continue; // Skip tags such as [anon:libc_malloc], [stack], [vectors], etc.

		// We only care about shared libraries - Android maps lots of .ttf s.  This also lets us skip a bunch of other weird "paths":
		//     /data/dalvik-cache/arm/data@app@com.maulingmonkey.debug.stack.nvidiaCodeworksTest-1@base.apk@classes.dex
		//     /dev/ashmem/dalvik-main space (deleted)
		//     etc.
		if (strcmp(".so", path + strlen(path) - sizeof(".so") + 1) != 0) continue;

		// .so end up mapped into the process multiple times (presumably different permissions for different segments?)
		// Abuse the fact that these are laid out sequentially to deduplicate:
		if (strcmp(prevPath,path) == 0) continue;
		memcpy(prevPath, path, sizeof(path)); static_assert(sizeof(prevPath) == sizeof(path), "prevPath and path expected to be the same size");

		mmkDebugModule m = {};
		m.path = path;

		m.name = path;
		if (const char* s = strrchr(m.name, '/'))  m.name = s+1;
		if (const char* s = strrchr(m.name, '\\')) m.name = s+1;

		if (!useNull) {
			if (!m.path   ) m.path    = "N/A";
			if (!m.name   ) m.name    = "N/A";
			if (!m.version) m.version = "N/A";
		}

		if (!(*onModule)(userData, &m)) break;
	}

	fclose(f);
}

#endif
