#pragma once

#include <chrono>

namespace Netcode {

	using ClockType = std::chrono::high_resolution_clock;
	using Timestamp = ClockType::time_point;
	using Duration = Timestamp::duration;
	
}
