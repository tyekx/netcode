#pragma once

#ifdef _DEBUG
/*
* In the debug version of the build, we include the option to observe each gameobject, this is unneccesary in a release build,
* as we only need it to update the UI in the engine
*/

#define EGG_DEBUG_ENGINE_DEFINE_OBSERVER(type)		\
type * __debugObserver;								\
public:												\
void SetObserver( type * obs ) {					\
	__debugObserver = obs;							\
}

#define EGG_DEBUG_ENGINE_CONSTRUCT_OBSERVER			\
, __debugObserver { nullptr }

#define EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(func, ...)	\
if(__debugObserver != nullptr)						\
__debugObserver-> func (__VA_ARGS__)

#else

/*
* If the project is in release, then we empty the debug functions out
*/
#define EGG_DEBUG_ENGINE_DEFINE_OBSERVER(type)
#define EGG_DEBUG_ENGINE_NOTIFY_OBSERVER(func, ...)
#define EGG_DEBUG_ENGINE_CONSTRUCT_OBSERVER

#endif
