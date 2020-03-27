#include "Sleep.h"

#ifdef _MSC_VER
	#include <Windows.h>
#else 
	#include <unistd.h>
#endif

void netcode::Sleep(unsigned int sleepForMs) {
	#ifdef _MSC_VER
	::Sleep(sleepForMs);
	#else
	useconds_t microseconds = ((useconds_t)sleepForMs) * 1000ULL;
	::usleep(microseconds);
	#endif
}
