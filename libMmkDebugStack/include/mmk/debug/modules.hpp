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

#ifndef ZMMK_IG_DEBUG_MODULES_HPP
#define ZMMK_IG_DEBUG_MODULES_HPP

#include "modules.h"

namespace mmk { namespace debug {
	typedef ::mmkDebugModule module;

	namespace stackInternal {
		template < typename fOnModule >
		int invokeOnModule(void* userData, const module* m) { return (*static_cast<fOnModule*>(userData))(*m) ? 1 : 0; }
	} // namespace stackInternal

	template < typename fOnModule >
	void modulesLoaded(
		mmkDebugModulesResolveFlags     flags,
		const fOnModule&                onModule) // ( const mmk::debug::module& module )
	{
		using namespace mmk::debug::stackInternal;
		::mmkDebugModulesLoaded( flags, const_cast<fOnModule*>(&onModule), &invokeOnModule<const fOnModule> );
	}
}} // namespace mmk::debug

#endif /* ndef ZMMK_IG_DEBUG_MODULES_HPP */
