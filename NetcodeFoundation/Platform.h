#pragma once

#if defined(_WIN32)
#define NETCODE_OS_WINDOWS
#include <sdkddkver.h>
#endif

#if defined(__linux__)
#define NETCODE_OS_LINUX
#endif

#if defined(_DEBUG)
#define NETCODE_DEBUG
#endif

