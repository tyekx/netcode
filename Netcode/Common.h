#pragma once

#include <NetcodeFoundation/Foundation.h>
#include <vector>
#include <memory>
#include "Logger.h"

namespace Egg::Internal {

	void Assert(bool trueMeansOk, const char * msgOnFail, ...);

}

#if defined(EGG_DEBUG)
#define ASSERT(trueMeansOk, msgOnFail, ...) Egg::Internal::Assert(trueMeansOk, msgOnFail, __VA_ARGS__)
#else 
#define ASSERT(trueMeansOk, msgOnFail, ...)
#endif
