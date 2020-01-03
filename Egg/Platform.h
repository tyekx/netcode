#pragma once

#if defined(_WIN32)
#define EGG_OS_WINDOWS
#endif

#if defined(__linux__)
#define EGG_OS_LINUX
#endif

#if defined(_DEBUG)
#define EGG_DEBUG
#endif

