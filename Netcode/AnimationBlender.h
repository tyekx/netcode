#pragma once

#include "AnimationState.h"
#include <NetcodeAssetLib/Bone.h>

namespace Netcode::Animation {
	class AnimationBlender {
		struct BoneSRT {
			DirectX::XMVECTOR translation;
			DirectX::XMVECTOR rotation;
			DirectX::XMVECTOR scale;
		};

		constexpr static int MAX_ACTIVE_STATE_COUNT = 8;

		BoneSRT buffer[128];
		DirectX::XMFLOAT4X4A toRootTransform[128];
		DirectX::XMFLOAT4X4A bindTransform[128];
		AnimationState * activeStates[MAX_ACTIVE_STATE_COUNT];
		unsigned int numActiveStates;
		Asset::Bone * bones;
		unsigned int bonesLength;
	public:

		AnimationBlender(Asset::Bone * skeleton, unsigned int skeletonLength);

		void ActivateState(AnimationState * state);

		void Blend();

		void UpdateStates(float dt);

		void CopyBoneDataInto(void * dest);
		void CopyToRootDataInto(void * dest);
	};

}
