#include "Stopwatch.h"

void Egg::Stopwatch::Start() {
	timestampStart = clock_type::now();
}

void Egg::Stopwatch::Stop() {
	timestampEnd = clock_type::now();
}

float Egg::Stopwatch::Restart() {
	Stop();
	float value = GetElapsedSeconds();
	Start();
	return value;
}

void Egg::Stopwatch::Reset() {
	Start();
	Stop();
}

float Egg::Stopwatch::GetElapsedSeconds() {
	return std::chrono::duration<float>(timestampEnd - timestampStart).count();
}
