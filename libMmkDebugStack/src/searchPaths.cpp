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

#include "searchPaths.hpp"
#include "assert.hpp"
#include <mmk/debug/stack.h>

namespace mmk { namespace debug {
	std::vector<std::string> stackSearchPaths;
}} // namespace mmk::debug

using namespace mmk::debug;

void mmkDebugStackSearchPaths(const char** paths, size_t pathsCount)
{
	MMK_DEBUG_STACK_LOCK_SCOPE();

	if (!ZMMK_ASSERT(pathsCount < 1000, "mmkDebugStackSearchPaths: Absurd number of search paths")) return;
	stackSearchPaths.resize(0);
	stackSearchPaths.reserve(pathsCount);

	while (pathsCount --> 0)
	{
		if (!*paths) break; // We treat a NULL as an alternative way to end the list of paths.
		stackSearchPaths.push_back(*paths);
		++paths;
	}
}
