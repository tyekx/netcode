#pragma once

#include "TimeTypes.h"

namespace Netcode {

	class GameClock {
		Duration fixedUpdateInterval;
		Duration fixedUpdateCache;
		Duration fixedUpdateAcc;
		Duration deltaTime;
		Duration deltaRtt;
		Duration thetaClockOffset;
		Duration totalTimeSinceEpoch;
		Duration epoch;
		Timestamp localFrameTimestamp;
		Timestamp globalFrameTimestamp;
	public:
		GameClock();
		
		void SetEpoch(Duration duration);
		
		void SynchronizeClocks(Duration delta, Duration theta);

		void SetFixedUpdateInterval(Duration d);

		[[nodiscard]]
		Duration GetFixedDeltaTime() const;

		[[nodiscard]]
		Timestamp GetLocalTime() const;

		[[nodiscard]]
		Timestamp GetGlobalTime() const;

		[[nodiscard]]
		Duration GetDeltaTime() const;

		[[nodiscard]]
		float FGetDeltaTime() const;

		[[nodiscard]]
		float FGetGlobalTotalTime() const;

		[[nodiscard]]
		float FGetTotalTime() const;

		// in seconds
		[[nodiscard]]
		double DGetGlobalTotalTime() const;
		
		// in seconds
		[[nodiscard]]
		double DGetDeltaTime() const;

		[[nodiscard]]
		double DGetTotalTime() const;

		void Tick();
	};
	
}
