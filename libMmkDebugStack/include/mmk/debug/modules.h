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

#ifndef ZMMK_IG_DEBUG_MODULES_H
#define ZMMK_IG_DEBUG_MODULES_H

#ifdef __cplusplus
#define ZMMK_DEBUG_MODULES_EXTERN_C extern "C"
#else
#define ZMMK_DEBUG_MODULES_EXTERN_C
#endif

#define ZMMK_DEBUG_MODULES_API  ZMMK_DEBUG_MODULES_EXTERN_C

enum mmkDebugModulesResolveFlags {
	/* Misc. options */
	mmkDebugModulesResolveNullMissing   = (1u << 0),  /* Use null (const char*)s for unresolveable information, instead of empty/placeholder strings.  Presumably the caller wants to use their own empty/placeholder strings. */

	/* Resolution options */
	mmkDebugModulesResolvePath          = (1u << 1), /* Attempt to lookup e.g. full .dll path info */
	mmkDebugModulesResolveVersion       = (1u << 2), /* Attempt to lookup e.g. .dll version information */

	/* Common sets */
	mmkDebugModulesResolveAll           = ~0u
	    & ~mmkDebugModulesResolveNullMissing
};

typedef struct {
	const char*     path;
	const char*     name;
	const char*     version; /* May be null (e.g. android/linux ELF files are only versioned by naming convention) */
} mmkDebugModule;

ZMMK_DEBUG_MODULES_API
void mmkDebugModulesLoaded(
	mmkDebugModulesResolveFlags    flags,
	void*                          userData,
	int                            (*onModule)(void* userData, const mmkDebugModule*));



#ifdef __cplusplus
#include "modules.hpp"
#endif

#endif /* ndef ZMMK_IG_DEBUG_MODULES_H */
