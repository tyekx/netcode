#pragma once

#include <memory>
#include <vector>
#include "Logger.h"

#ifdef _WIN32
#define EGG_OS_WINDOWS
#endif

#ifdef __linux__
#define EGG_OS_LINUX
#include "LinuxSal.h"
#endif


namespace Egg::Internal {

	void Assert(bool trueMeansOk, const char * msgOnFail, ...);

}

#define ASSERT(trueMeansOk, msgOnFail, ...) Egg::Internal::Assert(trueMeansOk, msgOnFail, __VA_ARGS__)

