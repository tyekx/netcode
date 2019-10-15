#pragma once


#include <DirectXMath.h>

namespace Egg {

	namespace Asset {

		enum class AnimationState {
			DEFAULT = 0,
			CONSTANT = 1,
			LINEAR = 2,
			REPEAT = 3
		};

		struct AnimationKey {
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT4 rotation;
			DirectX::XMFLOAT3 scale;
		};


		struct Animation {
			char name[56];

			double duration;
			double ticksPerSecond;

			unsigned int keysLength;
			unsigned int bonesLength;

			/*
			* Length of this array is bonesLength,
			* for each bone we have a pre and a post state
			*/
			AnimationState * preStates;
			AnimationState * postStates;

			/*
			* Length of this array is keysLength
			* for each animation key we have a timestamp
			*/
			double * times;

			/*
			* This is a linearized structure, to access a row:
			* keys + (bonesLength * i) will give the i-th row containing bonesLength of useful data.
			* Therefore i is expected to be between [0, keysLength-1]
			*/
			AnimationKey * keys;
		};

	}

}
