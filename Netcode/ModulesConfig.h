#pragma once

#include "Common.h"

/*
with build time definitions you can control the compiled modules. By default every buildable module is included in the build.

Define
NETCODE_BUILD_<name>_MODULE: to explicitly build a module, if its not possible, the build will not succeed
NETCODE_EXCLUDE_<name>_MODULE: to explicitly exclude a module from the build

Not having any built module for a specific type of modules is not considered to be an error, as every interface has an empty "Null" implementation to fall back to.
*/

#if !defined(NETCODE_EXCLUDE_DX12_MODULE)
#define DETAIL_BUILD_DX12_MODULE
#endif

#if defined(DETAIL_BUILD_DX12_MODULE)

	#if defined(NETCODE_OS_WINDOWS) && !defined(NETCODE_EXCLUDE_DX12_MODULE)

		#define NETCODE_BUILD_DX12_MODULE

	#else

		#pragma warning "The platform is not windows, DX12 module will be ommitted"

	#endif

#endif
