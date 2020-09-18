#pragma once

#include "TimeTypes.h"

namespace Netcode {

	Timestamp ConvertUInt64ToTimestamp(uint64_t v);
	uint64_t ConvertTimestampToUInt64(const Timestamp & timestamp);
	
	class SystemClock {
		~SystemClock() = default;
		SystemClock() = default;
		
		static Duration theta;
		static Duration delta;
		static Duration elapsedTimeSinceLastFrame;
		static Timestamp currentFrameTimestamp;
		static Timestamp processStartedAt;
	public:

		static void SynchronizeClocks(Duration delta, Duration theta);

		static void Tick();

		[[nodiscard]]
		static Timestamp LocalNow();

		[[nodiscard]]
		static Timestamp GlobalNow();
		
		[[nodiscard]]
		static float GetDeltaTimeInSeconds();
		
		[[nodiscard]]
		static double GetTotalTimeInSeconds();
	};

	/*
	 * precise sleep function which is a busy wait for the last < 16ms of the wait time
	 */
	void SleepFor(const Duration & duration);


	/*
	 * wait for time to pass while doing something on the side
	 */
	template<typename T>
	void BusyWait(const Duration & duration, T callable) {
		const Timestamp t = SystemClock::LocalNow();
		Duration tmp = std::chrono::seconds(0);
		while(tmp < duration) {
			callable();
			tmp = SystemClock::LocalNow() - t;
		}
	}
	
}

