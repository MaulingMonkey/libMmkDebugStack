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

#ifndef ZMMK_IG_DEBUG_STACK_H
#define ZMMK_IG_DEBUG_STACK_H

#include <stddef.h>

/* Public   APIs start with e.g.  "MMK_DEBUG_STACK_",  "mmkDebugStack", or "mmk::debug::"
 * Internal APIs start with e.g. "ZMMK_DEBUG_STACK_", "zmmkDebugStack", or "mmk::debug::stackInternal::"
 */

#ifdef __cplusplus
#define ZMMK_DEBUG_STACK_EXTERN_C extern "C"
#define ZMMK_DEBUG_STACK_IF_CPP(x) x
#else
#define ZMMK_DEBUG_STACK_EXTERN_C
#define ZMMK_DEBUG_STACK_IF_CPP(x)
#endif

#ifdef _MSC_VER
#define ZMMK_DEBUG_STACK_NO_INLINE      __declspec(noinline)
#define ZMMK_DEBUG_STACK_FORCE_INLINE   __forceinline
#else
#define ZMMK_DEBUG_STACK_NO_INLINE
#define ZMMK_DEBUG_STACK_FORCE_INLINE   inline
#endif

#ifdef __PRETTY_FUNCTION__
#define MMK_DEBUG_STACK_HINT() __FILE__, __LINE__, __PRETTY_FUNCTION__
#else
#define MMK_DEBUG_STACK_HINT() __FILE__, __LINE__, __FUNCTION__
#endif

#define ZMMK_DEBUG_STACK_API   ZMMK_DEBUG_STACK_EXTERN_C



/* Initialize mmkDebugStack.
 */
ZMMK_DEBUG_STACK_API void  mmkDebugStackInit(void);
ZMMK_DEBUG_STACK_API void zmmkDebugStackInitNoLock(void);
ZMMK_DEBUG_STACK_API void  mmkDebugStackSearchPaths(const char** paths, size_t pathsCount);



/* Some libraries mmkDebugStack* uses are not thread safe:
 *     dbgHelp.lib
 *     ???
 * Use these to protect access in C code.
 * For C++ code, prefer MMK_DEBUG_STACK_LOCK_SCOPE() which is RAII based and thus exception/early-exit safe.
 */
ZMMK_DEBUG_STACK_API void mmkDebugStackLock(void);
ZMMK_DEBUG_STACK_API void mmkDebugStackUnlock(void);



enum mmkDebugStackResolveFlags {
	/* Misc. options */
	mmkDebugStackResolveNullMissing  = (1u << 0),  /* Use null (const char*)s for unresolveable information, instead of empty/placeholder strings.  Presumably the caller wants to use their own empty/placeholder strings. */

	/* ResolveFunc - Controls how mmkDebugStackFunction is populated */
	mmkDebugStackResolveFuncName     = (1u << 1),  /* Populate mmkDebugStackFunction::name   */
	mmkDebugStackResolveFuncFile     = (1u << 2),  /* Populate mmkDebugStackFunction::file   */
	mmkDebugStackResolveFuncLine     = (1u << 3),  /* Populate mmkDebugStackFunction::line   */
	mmkDebugStackResolveFuncModule   = (1u << 4),  /* Populate mmkDebugStackFunction::module */
	mmkDebugStackResolveFuncHints    = (1u << 5),  /* Look at {file,line,func}Hint and attempt to fix up an inaccurate mmkDebugStackFunction::line based on that information. */

	/* ResolveVar - Controls how mmkDebugStackVariable is populated */
	mmkDebugStackResolveVarName      = (1u << 6),  /* Populate mmkDebugStackVariable::name  */
	mmkDebugStackResolveVarValue     = (1u << 7),  /* Populate mmkDebugStackVariable::value */
	mmkDebugStackResolveVarType      = (1u << 8),  /* Populate mmkDebugStackVariable::type  */

	/* Common sets */
	mmkDebugStackResolveNone         = 0u,         /* You just want addresses collected as quickly as possible.            */
	mmkDebugStackResolveAll          = ~0u         /* You want as much debug information resolved immediately as possible. */
	    & ~mmkDebugStackResolveNullMissing         /*     ... you still have to opt into getting NULL values however.      */
};

typedef struct {
	const void*     functionBase; /* May be null */
	const void*     address;      /* May be null (for placeholder psuedo-frames, and e.g. the fallback path where the caller file/line/func is our only information.) */
	const char*     name;         /* May be null */
	const char*     file;         /* May be null */
	const void*     moduleBase;   /* May be null */
	const char*     module;       /* May be null */
	unsigned        line;         /* May be 0 */
} mmkDebugStackFunction;

typedef struct {
	const mmkDebugStackFunction*    function;   /* Mustn't be null */
	const void*                     address;    /* May be null (if e.g. in a register) */
	const char*                     name;       /* May be null? */
	const char*                     value;      /* May be null */
	const char*                     type;       /* May be null */
} mmkDebugStackVariable;

ZMMK_DEBUG_STACK_API
ZMMK_DEBUG_STACK_NO_INLINE
void mmkDebugStackCurrentThread(
	mmkDebugStackResolveFlags   flags,
	unsigned                    framesToSkip,
	unsigned                    maxFrames,
	void*                       userData,
	int                         (*onFunction) (void* userData, const mmkDebugStackFunction*),
	int                         (*onVariable) (void* userData, const mmkDebugStackVariable*),
	/* Consider invoking MMK_DEBUG_STACK_HINT() to pass these values */
	const char*                 fileHint = 0,
	unsigned                    lineHint = 0,
	const char*                 funcHint = 0);



#ifdef __cplusplus
#include "stack.hpp"
#endif

#endif /* ndef ZMMK_IG_DEBUG_STACK_H */
