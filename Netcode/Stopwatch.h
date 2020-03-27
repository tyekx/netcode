#pragma once

#include <chrono>

namespace Netcode {

	class Stopwatch {
		using clock_type = std::chrono::high_resolution_clock;
		std::chrono::time_point<clock_type> timestampStart;
		std::chrono::time_point<clock_type> timestampEnd;

	public:
		void Start();
		void Stop();
		float Restart();
		void Reset();
		float GetElapsedSeconds();
	};

}
