#include "Stopwatch.h"
#include "System/SystemClock.h"

namespace Netcode {

	void Stopwatch::Start() {
		start = SystemClock::LocalNow();
	}

	void Stopwatch::Stop() {
		end = SystemClock::LocalNow();
	}

	float Stopwatch::Restart() {
		Stop();
		float value = FGetElapsedSeconds();
		Start();
		return value;
	}

	void Stopwatch::Reset() {
		Start();
		Stop();
	}

	float Stopwatch::FGetElapsedSeconds() {
		return std::chrono::duration<float>(end - start).count();
	}

}
