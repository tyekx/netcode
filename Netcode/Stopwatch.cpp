#include "Stopwatch.h"

void Netcode::Stopwatch::Start() {
	timestampStart = clock_type::now();
}

void Netcode::Stopwatch::Stop() {
	timestampEnd = clock_type::now();
}

float Netcode::Stopwatch::Restart() {
	Stop();
	float value = GetElapsedSeconds();
	Start();
	return value;
}

void Netcode::Stopwatch::Reset() {
	Start();
	Stop();
}

float Netcode::Stopwatch::GetElapsedSeconds() {
	return std::chrono::duration<float>(timestampEnd - timestampStart).count();
}
