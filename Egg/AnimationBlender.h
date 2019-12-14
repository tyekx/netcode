#pragma once

#include "AnimationState.h"
#include "Asset/Bone.h"

namespace Egg::Animation {
	class AnimationBlender {
		struct BoneSRT {
			DirectX::XMVECTOR translation;
			DirectX::XMVECTOR rotation;
			DirectX::XMVECTOR scale;
		};

		constexpr static int MAX_ACTIVE_STATE_COUNT = 8;

		BoneSRT buffer[128];
		AnimationState * activeStates[MAX_ACTIVE_STATE_COUNT];
		unsigned int numActiveStates;
		Asset::Bone * bones;
		unsigned int bonesLength;
		void * dest;
	public:

		AnimationBlender(Asset::Bone * skeleton, unsigned int skeletonLength, void * boneDataCb);

		void ActivateState(AnimationState * state);

		void Blend();

		void UpdateStates(float dt);

	};

}
