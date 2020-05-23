#pragma once

#include <NetcodeFoundation/Math.h>
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
			Netcode::Vector3 translation;
			Netcode::Quaternion rotation;
			Netcode::Vector3 scale;
		};

		std::vector<BlendItem> items;
		BoneSRT buffer[128];
		float wSum;
		void FillFrameData(const Asset::Animation & clip, const StateBase & state, BlendItem & item);

		void BlendState(uint32_t stateId, ArrayView<Asset::Animation> clips);
	public:
		void UpdatePlan(ArrayView<Asset::Animation> clips, ArrayView<StateBase> activeStates);

		const std::vector<BlendItem> & GetPlan() const;

		void Blend( ArrayView<Asset::Bone> bones,
					ArrayView<Asset::Animation> clips,
					Netcode::Float4x4 * toRootMatrices,
					Netcode::Float4x4 * bindMatrices );
	};

}
