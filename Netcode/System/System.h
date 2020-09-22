#pragma once

#include "SystemClock.h"

namespace Netcode {

	void Initialize();

	/*
	* precise sleep function
	*/
	void SleepFor(const Duration & duration);


	/*
	 * wait for time to pass while doing something on the side
	 */
	template<typename T>
	void BusyWait(const Duration & duration, T && callable) {
		const Timestamp t = SystemClock::LocalNow();
		Duration tmp = std::chrono::seconds(0);
		while(tmp < duration) {
			callable();
			tmp = SystemClock::LocalNow() - t;
		}
	}
	
}
