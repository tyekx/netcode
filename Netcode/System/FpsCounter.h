#pragma once

#include "TimeTypes.h"

namespace Netcode {

	class FrameCounter {
		constexpr static size_t HISTORY_BUFFER_SIZE = 128;
		
		Duration history[HISTORY_BUFFER_SIZE];
		Duration sum;
		size_t tickIdx;
		double avgSecondsPerFrame;
		double avgFramesPerSecond;
	public:
		FrameCounter();

		void Update(const Duration & lastFrameTime) noexcept;

		[[nodiscard]]
		double GetAvgFramesPerSecond() const noexcept;

		[[nodiscard]]
		double GetAvgSecondsPerFrame() const noexcept;
	};

	
}
