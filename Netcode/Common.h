#pragma once

#include <NetcodeFoundation/Foundation.h>
#include <vector>
#include <memory>
#include "Logger.h"

namespace Netcode::Detail {

	void NotImplementedAssertion();

	void Assert(bool trueMeansOk, const char * msgOnFail, ...);

}

#if defined(EGG_DEBUG)
#define ASSERT(trueMeansOk, msgOnFail, ...) Netcode::Detail::Assert(trueMeansOk, msgOnFail, __VA_ARGS__)
#else 
#define ASSERT(trueMeansOk, msgOnFail, ...)
#endif
