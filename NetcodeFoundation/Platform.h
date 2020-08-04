#pragma once

#if defined(_WIN32)
#define NETCODE_OS_WINDOWS
#endif

#if defined(__linux__)
#define NETCODE_OS_LINUX
#endif

#if defined(_DEBUG)
#define NETCODE_DEBUG
#endif

#if defined(_RELEASE) 
#define NETCODE_RELEASE
#endif

