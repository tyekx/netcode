#include "SystemClock.h"

#include <thread>

namespace Netcode {

	Duration SystemClock::theta;
	Duration SystemClock::delta;
	Timestamp SystemClock::currentFrameTimestamp;
	Timestamp SystemClock::processStartedAt = ClockType::now();
	
	void SystemClock::SynchronizeClocks(Duration delta, Duration theta) {
		SystemClock::theta = theta;
		SystemClock::delta = delta;
	}

	[[nodiscard]]
	Timestamp SystemClock::LocalNow() {
		return Timestamp{ ClockType::now() - processStartedAt };
	}

	[[nodiscard]]
	Timestamp SystemClock::GlobalNow() {
		return Timestamp{ (ClockType::now() - processStartedAt) + theta + (delta / 2) };
	}

	Timestamp ConvertUInt64ToTimestamp(uint64_t v) {
		return Timestamp{ Duration{ v } };
	}

	uint64_t ConvertTimestampToUInt64(const Timestamp & timestamp) {
		return timestamp.time_since_epoch().count();
	}

}
