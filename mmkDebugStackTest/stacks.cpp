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
#include <mmk/debug/stack.h>
#include <string.h>

// TODO: Move to mmk/debug/stack.h ?
#if defined _MSC_VER || defined __ANDROID__
#define MMK_DEBUG_STACK_IMPLEMENTS_STACKS
#endif

MMK_UNIT_TEST_CATEGORY("Stacks API")
{
	using namespace mmk::debug;

	MMK_UNIT_TEST("Returning false should stop enumeration") {
		int funcs = 0;
		stackCurrentThread( mmkDebugStackResolveAll, 0, 100, [&](const stackFunction& func) { (void)func; ++funcs; return false; } );
		ASSERT_CMP_MSG(funcs,<=,1, "Should never resolve more than 1 module if we just return false");
	}

	MMK_UNIT_TEST("Reaching maxFrames should stop enumeration") {
		int funcs = 0;
		stackCurrentThread( mmkDebugStackResolveAll, 0, 3, [&](const stackFunction& func) { (void)func; ++funcs; return true; } );
		ASSERT_CMP_MSG(funcs,<=,3, "Should never resolve more functions than maxFrames");
	}

	MMK_UNIT_TEST("Frames count SAN check") {
		int funcs = 0;
		stackCurrentThread( mmkDebugStackResolveAll, 0, 120, [&](const stackFunction& func) { (void)func; ++funcs; return true; } );
#ifdef MMK_DEBUG_STACK_IMPLEMENTS_STACKS
		ASSERT_CMP_MSG(funcs,>,1,  "Should resolve more than 1 function");
		ASSERT_CMP_MSG(funcs,<,100, "Should not resolve 100 functions, our stack isn't *that* deep");
#else // ndef MMK_DEBUG_STACK_IMPLEMENTS_MODULES
		ASSERT_CMP_MSG(funcs,==,0, "Should resolve no functions:  Supposedly not implemented, and we have no __FUNC__/__FILE__/__LINE__ hints.");
		stackCurrentThread( mmkDebugStackResolveAll, 0, 120, [&](const stackFunction& func) { (void)func; ++funcs; return true; }, MMK_DEBUG_STACK_HINT());
		ASSERT_CMP_MSG(funcs,==,1, "Should resolve one function:  Supposedly not implemented, but we have __FUNC__/__FILE__/__LINE__ hints to fake one frame.");
#endif
	}

	MMK_UNIT_TEST("Basic string flag invariants") {
		stackCurrentThread( mmkDebugStackResolveAll, 0, 100, [&](const stackFunction& func) {
			ASSERT_MSG(func.name  , "func.name"   " should never be null without mmkDebugModulesResolveNullMissing");
			ASSERT_MSG(func.file  , "func.file"   " should never be null without mmkDebugModulesResolveNullMissing");
			ASSERT_MSG(func.module, "func.module" " should never be null without mmkDebugModulesResolveNullMissing");
			return true;
		}, MMK_DEBUG_STACK_HINT());

		// TODO: Change type signature of modulesLoaded to some integer type, get rid of this cast
		stackCurrentThread( mmkDebugStackResolveFlags(mmkDebugStackResolveAll | mmkDebugStackResolveNullMissing), 0, 100, [&](const stackFunction& func) {
			if (func.name  ) ASSERT_FMT(strcmp(func.name  , "N/A") != 0, "func.name"   " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing", func.name  );
			if (func.file  ) ASSERT_FMT(strcmp(func.file  , "N/A") != 0, "func.file"   " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing", func.file  );
			if (func.module) ASSERT_FMT(strcmp(func.module, "N/A") != 0, "func.module" " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing", func.module);
			return true;
		}, MMK_DEBUG_STACK_HINT());

		stackCurrentThread( mmkDebugStackResolveNone, 0, 100, [&](const stackFunction& func) {
			ASSERT_MSG(func.name  , "func.name"   " should never be null without mmkDebugModulesResolveNullMissing, despite specifying no resolve flags");
			ASSERT_MSG(func.file  , "func.file"   " should never be null without mmkDebugModulesResolveNullMissing, despite specifying no resolve flags");
			ASSERT_MSG(func.module, "func.module" " should never be null without mmkDebugModulesResolveNullMissing, despite specifying no resolve flags");
			return true;
		}, MMK_DEBUG_STACK_HINT());

		stackCurrentThread( mmkDebugStackResolveNullMissing, 0, 100, [&](const stackFunction& func) {
			if (func.name  ) ASSERT_FMT(strcmp(func.name  , "N/A") != 0, "func.name"   " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing, despite specifying no other resolve flags", func.name  );
			if (func.file  ) ASSERT_FMT(strcmp(func.file  , "N/A") != 0, "func.file"   " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing, despite specifying no other resolve flags", func.file  );
			if (func.module) ASSERT_FMT(strcmp(func.module, "N/A") != 0, "func.module" " \"%s\" should never be 'N/A' with mmkDebugModulesResolveNullMissing, despite specifying no other resolve flags", func.module);
			return true;
		}, MMK_DEBUG_STACK_HINT());
	}
}
