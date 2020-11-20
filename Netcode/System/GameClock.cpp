#include "GameClock.h"
#include "SystemClock.h"

namespace Netcode {

	GameClock::GameClock() : fixedUpdateInterval{}, fixedUpdateCache{}, fixedUpdateAcc{},
		deltaTime{}, deltaRtt{}, thetaClockOffset{}, totalTimeSinceEpoch{},
		epoch{}, localFrameTimestamp{}, globalFrameTimestamp{} {

	}

	void GameClock::SetEpoch(Duration duration) {
		epoch = duration;
		localFrameTimestamp = Timestamp{ };
		globalFrameTimestamp = localFrameTimestamp + deltaRtt / 2 + thetaClockOffset;
		totalTimeSinceEpoch = Duration{ };
	}

	void GameClock::SynchronizeClocks(Duration delta, Duration theta) {
		deltaRtt = delta;
		thetaClockOffset = theta;
	}

	void GameClock::SetFixedUpdateInterval(Duration d) {
		fixedUpdateInterval = d;
	}

	[[nodiscard]]
	Duration GameClock::GetFixedDeltaTime() const {
		return fixedUpdateCache;
	}

	[[nodiscard]]
	Timestamp GameClock::GetLocalTime() const {
		return localFrameTimestamp;
	}

	[[nodiscard]]
	Timestamp GameClock::GetGlobalTime() const {
		return globalFrameTimestamp;
	}

	[[nodiscard]]
	Duration GameClock::GetDeltaTime() const {
		return deltaTime;
	}

	// in seconds
	[[nodiscard]]
	float GameClock::FGetDeltaTime() const {
		return std::chrono::duration<float>(deltaTime).count();
	}

	// in seconds
	[[nodiscard]]
	float GameClock::FGetGlobalTotalTime() const {
		return std::chrono::duration<float>(totalTimeSinceEpoch + deltaRtt / 2 + thetaClockOffset).count();
	}

	// in seconds
	[[nodiscard]]
	float GameClock::FGetTotalTime() const {
		return std::chrono::duration<float>(totalTimeSinceEpoch).count();
	}

	// in seconds
	[[nodiscard]]
	double GameClock::DGetGlobalTotalTime() const {
		return std::chrono::duration<double>(totalTimeSinceEpoch + deltaRtt / 2 + thetaClockOffset).count();
	}

	// in seconds
	[[nodiscard]]
	double GameClock::DGetDeltaTime() const {
		return std::chrono::duration<double>(deltaTime).count();
	}

	// in seconds
	[[nodiscard]]
	double GameClock::DGetTotalTime() const {
		return std::chrono::duration<double>(totalTimeSinceEpoch).count();
	}

	void GameClock::Tick() {
		const Timestamp newFrameTimestamp = SystemClock::LocalNow() - epoch;

		deltaTime = newFrameTimestamp - localFrameTimestamp;
		localFrameTimestamp = newFrameTimestamp;
		globalFrameTimestamp = localFrameTimestamp + deltaRtt / 2 + thetaClockOffset;

		fixedUpdateCache = Duration{ };

		fixedUpdateAcc += deltaTime;
		if(fixedUpdateAcc > fixedUpdateInterval) {
			fixedUpdateAcc -= fixedUpdateInterval;
			fixedUpdateCache = fixedUpdateInterval;
		}
	}
	
}
