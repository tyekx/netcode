#include "Common.h"
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>

#if defined(NETCODE_OS_WINDOWS)
#include <Windows.h>
#endif

#include "Logger.h"
#include <NetcodeFoundation/Exceptions.h>

namespace Netcode::Detail {

	static void LogString(const char * str) {
		Log::Error(str);
	}

	static void DebugBreak() {
#if defined(NETCODE_OS_WINDOWS)
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
