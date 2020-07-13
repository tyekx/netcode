#pragma once

#include <NetcodeFoundation/Foundation.h>
#include <vector>
#include <memory>
#include "Logger.h"

namespace Netcode::Detail {

	void Assert(bool trueMeansOk, const char * msgOnFail, ...);

}

#if defined(NETCODE_DEBUG)
#define ASSERT(trueMeansOk, msgOnFail, ...) Netcode::Detail::Assert(trueMeansOk, msgOnFail, __VA_ARGS__)
#else 
#define ASSERT(trueMeansOk, msgOnFail, ...) { trueMeansOk; }
#endif
