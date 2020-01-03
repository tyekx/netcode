#include "Common.h"
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

#if defined(EGG_OS_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include "Logger.h"

namespace Egg::Internal {

	static void LogString(const char * str) {
#if defined(EGG_OS_WINDOWS)
		Log::Error(str);
#endif
	}

	static void DebugBreak() {
#if defined(EGG_OS_WINDOWS)
		if(IsDebuggerPresent()) {
			__debugbreak();
		}
#endif
	}

	void Assert(bool trueMeansOk, const char * msgOnFail, ...) {
		if(!trueMeansOk) {
			va_list argList;
			va_start(argList, msgOnFail);

			std::string buffer;
			buffer.resize(1024);
			/*
			vsprintf_s:
			v: takes a va_list (variadic arg list)
			s: writes to string
			printf
			_s: secure, takes buffer size as argument
			*/
			vsprintf_s(&(buffer.at(0)), 1024, msgOnFail, argList);

			LogString(buffer.c_str());
			DebugBreak();
			exit(-1);

			va_end(argList);
		}

	}
}
