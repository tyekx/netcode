#pragma once

#include <NetcodeFoundation/Math.h>
#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeAssetLib/Animation.h>
#include <NetcodeAssetLib/Bone.h>
#include "BoneTransform.h"
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
		std::vector<BlendItem> items;
		BoneTransform buffer[128];
		float wSum;
		void FillFrameData(const Asset::Animation & clip, const StateBase & state, BlendItem & item);

		void BlendState(uint32_t stateId, ArrayView<Asset::Animation> clips);
	public:
		void UpdatePlan(ArrayView<Asset::Animation> clips, ArrayView<StateBase> activeStates);

		const std::vector<BlendItem> & GetPlan() const;

		void Blend(ArrayView<Asset::Animation> clips);

		MutableArrayView<BoneTransform> GetBoneTransforms();

		void UpdateMatrices(ArrayView<Asset::Bone> bones,
			Netcode::Float4x4 * toRootMatrices,
			Netcode::Float4x4 * bindMatrices);
	};

}
