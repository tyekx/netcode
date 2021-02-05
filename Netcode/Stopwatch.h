#pragma once

#include "System/TimeTypes.h"

namespace Netcode {

	class Stopwatch {
		Timestamp start;
		Timestamp end;

	public:
		void Start();
		void Stop();
		float Restart();
		void Reset();
		float FGetElapsedSeconds();
		
		Duration GetElapsedDuration() {
			return end - start;
		}
	};

}
