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
		static Timestamp currentFrameTimestamp;
		static Timestamp processStartedAt;
	public:

		static void SynchronizeClocks(Duration delta, Duration theta);

		/**
		 * Returns a new timestamp
		 */
		[[nodiscard]]
		static Timestamp LocalNow();

		/**
		 * Returns a new timestamp adjusted to the server's clock
		 */
		[[nodiscard]]
		static Timestamp GlobalNow();
	};
	
}

