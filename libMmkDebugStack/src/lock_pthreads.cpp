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

#ifndef __GNUC__
void lock_pthreads_exports_nothing() {} // Supress linker warning
#else

#include <mmk/debug/stack.hpp>
#include <pthread.h>

namespace
{
	pthread_mutex_t miscMutex;
}

void mmkDebugStackLock(void)
{
	pthread_mutex_lock(&miscMutex);
}

void mmkDebugStackUnlock(void)
{
	pthread_mutex_unlock(&miscMutex);
}

#endif
