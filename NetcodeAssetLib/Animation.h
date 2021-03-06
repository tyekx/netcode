#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode::Asset {

	enum class AnimationEdge {
		DEFAULT = 0,
		CONSTANT = 1,
		LINEAR = 2,
		REPEAT = 3
	};

	struct AnimationKey {
		Netcode::Float4 position;
		Netcode::Float4 rotation;
		Netcode::Float4 scale;
	};

	struct Animation {
		char name[56];

		float duration;
		float ticksPerSecond;

		uint32_t keysLength;
		uint32_t bonesLength;

		/*
		* Length of this array is keysLength
		* for each animation key we have a timestamp
		*/
		float * times;

		/*
		* This is a linearized structure, to access a row:
		* keys + (bonesLength * i) will give the i-th row containing bonesLength of useful data.
		* Therefore i is expected to be between [0, keysLength-1]
		*/
		AnimationKey * keys;
	};

}
