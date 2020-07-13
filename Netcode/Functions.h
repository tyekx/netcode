#pragma once

#include <cstdint>
#include <cmath>

namespace Netcode::Function {
	using Scalar = float (*)(float);

	float EaseIn(float x);

	float EaseOut(float x);

	float EaseOutQuad(float x);

	float LerpIn(float x);

	float LerpOut(float x);

	float HalfStep(float x);

	float ConstantZero(float x);

	float ConstantOne(float x);
}
