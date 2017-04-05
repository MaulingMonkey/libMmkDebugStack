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

#include <mmk/test/unit.hpp>
#include <mmk/debug/modules.h> // modulesLoaded etc.
#include <limits.h>
#include <string.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

// TODO: Move to mmk/debug/modules.h ?
#if defined _MSC_VER || defined __ANDROID__
#define MMK_DEBUG_STACK_IMPLEMENTS_MODULES
#endif

MMK_UNIT_TEST_CATEGORY("Modules API")
{
	using namespace mmk::debug;

	MMK_UNIT_TEST("Returning false should stop enumeration") {
		int modules = 0;
		modulesLoaded( mmkDebugModulesResolveAll, [&](const module& module) { (void)module; ++modules; return false; } );
		ASSERT_CMP_MSG(modules,<=,1, "Should never resolve more than 1 module if we just return false");
	}

	MMK_UNIT_TEST("Modules count SAN check") {
		int modules = 0;
		modulesLoaded( mmkDebugModulesResolveAll, [&](const module& module) { (void)module; ++modules; return true; } );
#ifdef MMK_DEBUG_STACK_IMPLEMENTS_MODULES
		ASSERT_CMP_MSG(modules,>,1, "Should resolve more than 1 module");
#else // ndef MMK_DEBUG_STACK_IMPLEMENTS_MODULES
		ASSERT_CMP_MSG(modules,==,0, "Shouldn't resolve any modules, supposedly not implemented for this platform yet");
#endif
	}

	MMK_UNIT_TEST("Basic string flag invariants") {
		modulesLoaded( mmkDebugModulesResolveAll, [&](const module& module) {
			ASSERT_MSG(module.name   , "module.name"    " should never be null without mmkDebugModulesResolveNullMissing");
			ASSERT_MSG(module.version, "module.version" " should never be null without mmkDebugModulesResolveNullMissing");
			ASSERT_MSG(module.path   , "module.path"    " should never be null without mmkDebugModulesResolveNullMissing");
			return true;
		});

		// TODO: Change type signature of modulesLoaded to some integer type, get rid of this cast
		modulesLoaded( mmkDebugModulesResolveFlags(mmkDebugModulesResolveAll | mmkDebugModulesResolveNullMissing), [&](const module& module) {
			if (module.name   ) ASSERT_FMT(strcmp(module.name,    "N/A") != 0, "module.name"    " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing", module.name   );
			if (module.version) ASSERT_FMT(strcmp(module.version, "N/A") != 0, "module.version" " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing", module.version);
			if (module.path   ) ASSERT_FMT(strcmp(module.path,    "N/A") != 0, "module.path"    " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing", module.path   );
			return true;
		});

		// TODO: Add None flag?
		modulesLoaded( mmkDebugModulesResolveFlags(0), [&](const module& module) {
			ASSERT_MSG(module.name   , "module.name"    " should never be null without mmkDebugModulesResolveNullMissing, despite specifying no resolve flags");
			ASSERT_MSG(module.version, "module.version" " should never be null without mmkDebugModulesResolveNullMissing, despite specifying no resolve flags");
			ASSERT_MSG(module.path   , "module.path"    " should never be null without mmkDebugModulesResolveNullMissing, despite specifying no resolve flags");
			return true;
		});

		modulesLoaded( mmkDebugModulesResolveFlags(mmkDebugModulesResolveNullMissing), [&](const module& module) {
			if (module.name   ) ASSERT_FMT(strcmp(module.name,    "N/A") != 0, "module.name"    " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing, despite specifying no other resolve flags", module.name   );
			if (module.version) ASSERT_FMT(strcmp(module.version, "N/A") != 0, "module.version" " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing, despite specifying no other resolve flags", module.version);
			if (module.path   ) ASSERT_FMT(strcmp(module.path,    "N/A") != 0, "module.path"    " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing, despite specifying no other resolve flags", module.path   );
			return true;
		});
	}

#ifdef _MSC_VER
	bool IsLocalAbsolutePath      (const char* path)                          { return path && path[0] && path[1] == ':' && path[2] == '\\'; }
	bool StartsWithCaseInsensitive(const char* s, const char* expectedPrefix) { return s && expectedPrefix && _strnicmp(s, expectedPrefix, strlen(expectedPrefix)) == 0; }
	bool EqualCaseInsensitive     (const char* a, const char* b)              { return (a==b) || (a && b && (_stricmp(a, b) == 0)); }

	bool IsSelfExe(const char* name) { return EqualCaseInsensitive(name, "mmkDebugStackTest"); }

	bool IsKnownWindowsDll(const char* name) {
		return EqualCaseInsensitive     (name, "ntdll")
			|| EqualCaseInsensitive     (name, "kernel32")
			|| EqualCaseInsensitive     (name, "kernelbase")
			|| EqualCaseInsensitive     (name, "dbghelp")
			|| EqualCaseInsensitive     (name, "msvcrt")
			|| EqualCaseInsensitive     (name, "version")
			|| StartsWithCaseInsensitive(name, "api-ms-win-");
	}

	MMK_UNIT_TEST("Windows name SAN checks") {
		int knownSelfExes    = 0;
		int knownWindowsDlls = 0;

		mmk::debug::modulesLoaded( mmkDebugModulesResolveFlags(mmkDebugModulesResolveAll | mmkDebugModulesResolveNullMissing), [&](const mmk::debug::module& module) {
			if (IsSelfExe(module.name))         ++knownSelfExes;
			if (IsKnownWindowsDll(module.name)) ++knownWindowsDlls;
			return true;
		});

		ASSERT_CMP_FMT(knownSelfExes,    ==, 1, "Expected only one instance of libMmkDebugStack to be linked (you didn't double-link it, right?)");
		ASSERT_CMP_FMT(knownWindowsDlls, >=, 3, "Expected at least a few known windows DLLs");
	}

	MMK_UNIT_TEST("Windows path SAN checks") {
		ASSERT(SetCurrentDirectoryA("\\"));
		mmk::debug::modulesLoaded( mmkDebugModulesResolveFlags(mmkDebugModulesResolveAll | mmkDebugModulesResolveNullMissing), [&](const mmk::debug::module& module) {
			// I could see non-windows modules being loaded from UNC paths which I don't yet handle... should still be absolute though
			if (IsKnownWindowsDll(module.name)) ASSERT_FMT(IsLocalAbsolutePath(module.path), "Module path \"%s\" should be an absolute path for known windows DLLs", module.path);
			if (module.path) {
				const DWORD attrs = GetFileAttributesA(module.path);
				if (attrs == INVALID_FILE_ATTRIBUTES) ASSERT_FMT(GetLastError() != ERROR_FILE_NOT_FOUND, "Module path \"%s\" does not exist!",          module.path);
				else                                  ASSERT_FMT(!(attrs & FILE_ATTRIBUTE_DIRECTORY),    "Module path \"%s\" is actually a directory!", module.path);
			}
			return true;
		});
	}

	MMK_UNIT_TEST("Windows versioning SAN checks") {
		mmk::debug::modulesLoaded( mmkDebugModulesResolveFlags(mmkDebugModulesResolveAll | mmkDebugModulesResolveNullMissing), [&](const mmk::debug::module& module) {
			const bool isKnownSelf       = IsSelfExe(module.name);
			const bool isKnownWindowsDll = IsKnownWindowsDll(module.name);

			ASSERT_MSG(!isKnownSelf       || !module.version, "Version should be NULL for libMmkDebugStack (we don't embed version information yet)");
			ASSERT_MSG(!isKnownWindowsDll ||  module.version, "Version shouldn't be NULL for known windows DLLs");

			if (module.version) {
				int a=0, b=0, c=0, d=0;
				char s[2] = "";
				ASSERT_FMT(module.version && (sscanf_s(module.version, "%d.%d.%d.%d%1s", &a, &b, &c, &d, s, (unsigned)_countof(s)) == 4), "Version \"%s\" should match a.b.c.d format on Windows", module.version);

				if (module.version) {
					ASSERT_CMP_FMT(a,<=,USHRT_MAX, "Version \"%s\" A component (%d) shouldn't exceed USHRT_MAX (%d)", module.version, a, USHRT_MAX);
					ASSERT_CMP_FMT(b,<=,USHRT_MAX, "Version \"%s\" B component (%d) shouldn't exceed USHRT_MAX (%d)", module.version, b, USHRT_MAX);
					ASSERT_CMP_FMT(c,<=,USHRT_MAX, "Version \"%s\" C component (%d) shouldn't exceed USHRT_MAX (%d)", module.version, c, USHRT_MAX);
					ASSERT_CMP_FMT(d,<=,USHRT_MAX, "Version \"%s\" D component (%d) shouldn't exceed USHRT_MAX (%d)", module.version, d, USHRT_MAX);
				}

				if (isKnownWindowsDll) {
					// Some typical windows DLL versions:
					// 6.1.7601.23677
					// 14.0.24210.0
					// 10.0.10586.9
					// 6.1.7601.17514
					// 7.0.7601.17744
					// 6.1.7600.16385
					ASSERT_CMP_FMT(a,<,100, "Version \"%s\" A/Major component (%d) shouldn't be huge (<100) for known Windows DLLs", module.version, a);
					ASSERT_CMP_FMT(b,<,100, "Version \"%s\" B/Minor component (%d) shouldn't be huge (<100) for known Windows DLLs", module.version, b);
					ASSERT_CMP_FMT(c,>,100, "Version \"%s\" C/Build component (%d) is typically huge (>100) for known Windows DLLs", module.version, c);
					// These tests could technically generate false positives:
					//     Windows versioning policy could change
					//     Unversioned proxy DLLs could be used for legitimate intercepting purpouses
					//     Unversioned proxy DLLs could be used for malware
					// However, I think these scenarios would be unusual/rare enough to be worth reviewing manually.
				}
			}

			return true;
		});
	}
#endif
}
