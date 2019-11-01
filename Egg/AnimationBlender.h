#pragma once

#include "AnimationState.h"
#include "ConstantBufferTypes.h"
#include "Asset/Bone.h"

namespace Egg::Animation {
	class AnimationBlender {
		struct BoneSRT {
			DirectX::XMVECTOR translation;
			DirectX::XMVECTOR rotation;
			DirectX::XMVECTOR scale;
		};

		constexpr static int MAX_ACTIVE_STATE_COUNT = 8;

		BoneSRT buffer[BoneDataCb::MAX_BONE_COUNT];
		AnimationState * activeStates[MAX_ACTIVE_STATE_COUNT];
		UINT numActiveStates;
		Asset::Bone * bones;
		UINT bonesLength;
		BoneDataCb * dest;
	public:

		AnimationBlender(Asset::Bone * skeleton, UINT skeletonLength, BoneDataCb * boneDataCb);

		void ActivateState(AnimationState * state);

		void Blend();

		void UpdateStates(float dt);

	};

}
