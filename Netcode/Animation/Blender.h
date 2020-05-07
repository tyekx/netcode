#pragma once

#include <DirectXMath.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeAssetLib/Animation.h>
#include <NetcodeAssetLib/Bone.h>
#include <tuple>
#include "State.h"

namespace Netcode::Animation {

	struct BlendItem {
		float weight;
		float normalizedTime;
		uint32_t beginFrameIndex;
		uint32_t endFrameIndex;
		uint32_t clipId;
		BoneMask mask;
	};

	class Blender {
	public:
		struct BoneSRT {
			DirectX::XMVECTOR translation;
			DirectX::XMVECTOR rotation;
			DirectX::XMVECTOR scale;
		};

		std::vector<BlendItem> items;
		BoneSRT buffer[128];
		float wSum;
		void FillFrameData(Asset::Animation & clip, const StateBase & state, BlendItem & item);

		void BlendState(uint32_t stateId, ArrayView<Asset::Animation> clips);
	public:
		void UpdatePlan(ArrayView<Asset::Animation> clips, ArrayView<StateBase> activeStates);

		const std::vector<BlendItem> & GetPlan() const;

		void Blend( ArrayView<Asset::Bone> bones,
					ArrayView<Asset::Animation> clips,
					DirectX::XMFLOAT4X4A * toRootMatrices,
					DirectX::XMFLOAT4X4A * bindMatrices );
	};

}
