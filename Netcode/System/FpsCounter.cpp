#include "FpsCounter.h"


namespace Netcode {

	FrameCounter::FrameCounter() : history{}, sum{}, tickIdx{ 0 }, avgSecondsPerFrame{ 0.0 }, avgFramesPerSecond{ 0.0 } { }

	void FrameCounter::Update(const Duration & lastFrameTime) noexcept {
		sum -= history[tickIdx];
		sum += lastFrameTime;
		history[tickIdx] = lastFrameTime;
		tickIdx = (tickIdx + 1) % HISTORY_BUFFER_SIZE;
		avgSecondsPerFrame = std::chrono::duration<double>(sum / HISTORY_BUFFER_SIZE).count();
		avgFramesPerSecond = 1.0 / avgSecondsPerFrame;
	}

	[[nodiscard]]
	double FrameCounter::GetAvgFramesPerSecond() const noexcept {
		return avgFramesPerSecond;
	}

	[[nodiscard]]
	double FrameCounter::GetAvgSecondsPerFrame() const noexcept {
		return avgSecondsPerFrame;
	}
	
}
