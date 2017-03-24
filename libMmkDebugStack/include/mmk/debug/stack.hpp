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

#ifndef ZMMK_IG_DEBUG_STACK_HPP
#define ZMMK_IG_DEBUG_STACK_HPP

#include "stack.h"

#define MMK_DEBUG_STACK_LOCK_SCOPE() ::mmk::debug::stackInternal::scopedLock zmmkDebugStackLock

namespace mmk { namespace debug {
	typedef ::mmkDebugStackFunction stackFunction;
	typedef ::mmkDebugStackVariable stackVariable;

	namespace stackInternal {
		// Used to implement MMK_DEBUG_STACK_LOCK_SCOPE
		struct scopedLock {
			scopedLock()	{ ::mmkDebugStackLock(); }
			~scopedLock()	{ ::mmkDebugStackUnlock(); }
		private:
			scopedLock(const scopedLock&);            // Uncopyable
			scopedLock& operator=(const scopedLock&); // Unassignable
		};

		// Used to implement stackCurrentThread
		template < typename fOnFunction, typename fOnVariable > struct cppEnumContext {
			const fOnFunction& onFunction;
			const fOnVariable& onVariable;

			typedef cppEnumContext<fOnFunction, fOnVariable> tSelf;
			static int invokeOnFunction(void* userData, const stackFunction* function) { return static_cast<tSelf*>(userData)->onFunction(*function) ? 1 : 0; }
			static int invokeOnVariable(void* userData, const stackVariable* variable) { return static_cast<tSelf*>(userData)->onVariable(*variable) ? 1 : 0; }
		};
	} // namespace stackInternal

	template < typename fOnFunction, typename fOnVariable >
	ZMMK_DEBUG_STACK_NO_INLINE // For consistent number of skipped frames
	void stackCurrentThread(
		mmkDebugStackResolveFlags	flags,
		unsigned					framesToSkip,
		unsigned					maxFrames,
		const fOnFunction&			onFunction, // bool ( const mmk::debug::stackFunction& function )
		const fOnVariable&			onVariable, // bool ( const mmk::debug::stackVariable& variable )
		// Consider invoking MMK_DEBUG_STACK_HINT() to pass these values
		const char*					fileHint = 0,
		unsigned					lineHint = 0,
		const char*					funcHint = 0)
	{
		using namespace mmk::debug::stackInternal;
		typedef cppEnumContext<fOnFunction, fOnVariable> tContext;
		tContext context = { onFunction, onVariable };
		::mmkDebugStackCurrentThread( flags, framesToSkip+1, maxFrames, &context, &tContext::invokeOnFunction, &tContext::invokeOnVariable, fileHint, lineHint, funcHint );
	}

	template < typename fOnFunction >
	ZMMK_DEBUG_STACK_NO_INLINE // For consistent number of skipped frames
	void stackCurrentThread(
		mmkDebugStackResolveFlags	flags,
		unsigned					framesToSkip,
		unsigned					maxFrames,
		const fOnFunction&			onFunction,		// ( const mmk::debug::stackFrameFunction& function )
		// Consider invoking MMK_DEBUG_STACK_HINT() to pass these values
		const char*					fileHint = 0,
		unsigned					lineHint = 0,
		const char*					funcHint = 0)
	{
		using namespace mmk::debug::stackInternal;
		typedef cppEnumContext<fOnFunction, int> tContext;
		tContext context = { onFunction, 0 };
		::mmkDebugStackCurrentThread( flags, framesToSkip+1, maxFrames, &context, &tContext::invokeOnFunction, 0, fileHint, lineHint, funcHint );
	}
}} // namespace mmk::debug

#endif /* ndef ZMMK_IG_DEBUG_STACK_HPP */
