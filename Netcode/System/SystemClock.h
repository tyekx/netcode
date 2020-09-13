#pragma once

#include <chrono>


namespace Netcode {

	using ClockType = std::chrono::high_resolution_clock;
	using Timestamp = ClockType::time_point;
	using Duration = Timestamp::duration;


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
	
}

