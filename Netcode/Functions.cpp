#include "Functions.h"

namespace Netcode::Function {
	float EaseIn(float x) {
		return x * x / (x * x + (1.0f - x) * (1.0f - x));
	}
	float EaseOut(float x) {
		return 1.0f - x * x / (x * x + (1.0f - x) * (1.0f - x));
	}

	float EaseOutQuad(float x)
	{
		return 1.0f - (1.0f - x) * (1.0f - x);
	}

	float LerpIn(float x) {
		return x;
	}
	float LerpOut(float x) {
		return 1.0f - x;
	}
	float HalfStep(float x)
	{
		if(x < 0.5f) {
			return 0.0f;
		} else {
			return 1.0f;
		}
	}
	float ConstantZero(float x) {
		return 0.0f;
	}
	float ConstantOne(float x) {
		return 1.0f;
	}
}
