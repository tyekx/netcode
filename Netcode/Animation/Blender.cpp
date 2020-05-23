#include "Blender.h"
#include <algorithm>
#include <Netcode/Logger.h>

namespace Netcode::Animation {
	void Blender::FillFrameData(const Asset::Animation & clip, const StateBase & state, BlendItem & item) {
		float t = state.time;
		unsigned int idx;
		for(idx = 1; idx < clip.keysLength; ++idx) {
			if(clip.times[idx - 1] <= t && clip.times[idx] >= t) {
				t = (t - clip.times[idx - 1]) / (clip.times[idx] - clip.times[idx - 1]);
				break;
			}
		}

		if(idx == clip.keysLength) {
			item.beginFrameIndex = clip.keysLength - 1;
			item.endFrameIndex = 0;
			item.normalizedTime = 0.0f;
		} else {
			item.beginFrameIndex = idx - 1;
			item.endFrameIndex = idx;
			item.normalizedTime = t;
		}

	}

	void Blender::UpdatePlan(ArrayView<Asset::Animation> clips, ArrayView<StateBase> activeStates) {
		items.clear();
		for(const StateBase & state : activeStates) {
			BlendItem item;
			item.clipId = static_cast<uint32_t>(state.clipId);
			item.weight = state.weight;
			item.mask = state.boneMask;
			FillFrameData(clips[item.clipId], state, item);

			items.push_back(item);
		}
	}
	const std::vector<BlendItem> & Blender::GetPlan() const {
		return items;
	}

	static BoneTransform BlendFrames(Netcode::Asset::AnimationKey * lhs, Netcode::Asset::AnimationKey * rhs, float t) {
		Netcode::Vector3 stPos;
		Netcode::Vector3 endPos;

		Netcode::Quaternion stQuat;
		Netcode::Quaternion endQuat;

		Netcode::Vector3 stScale;
		Netcode::Vector3 endScale;

		stPos = lhs->position;
		stQuat = lhs->rotation;
		stScale = lhs->scale;

		endPos = rhs->position;
		endQuat = rhs->rotation;
		endScale = rhs->scale;

		stPos = Netcode::Vector3::Lerp(stPos, endPos, t);
		stQuat = Netcode::Quaternion::Slerp(stQuat, endQuat, t);
		stScale = Netcode::Vector3::Lerp(stScale, endScale, t);

		BoneTransform srt;
		srt.translation = stPos;
		srt.rotation = stQuat;
		srt.scale = stScale;
		return srt;
	}

	void Blender::BlendState(uint32_t stateId, ArrayView<Asset::Animation> clips)
	{
		const BlendItem & item = items.at(stateId);

		float weight = item.weight;
		float t = item.normalizedTime;
		const Asset::Animation * a = clips.Data() + item.clipId;
		uint32_t idx0 = item.beginFrameIndex;
		uint32_t idx1 = item.endFrameIndex;
		auto * startKey = (a->keys + idx0 * a->bonesLength);
		auto * endKey = (a->keys + idx1 * a->bonesLength);

		for(unsigned int i = 0; i < a->bonesLength; ++i) {
			BoneTransform res = BlendFrames(startKey + i, endKey + i, t);

			float w = weight + wSum;
			float nw = weight / (weight + wSum);
			if(w < 0.0001f) {
				nw = 0.0f;
			}

			buffer[i].translation = Netcode::Vector3::Lerp(buffer[i].translation, res.translation, nw);
			buffer[i].rotation = Netcode::Quaternion::Slerp(buffer[i].rotation, res.rotation, nw);
			buffer[i].scale = Netcode::Vector3::Lerp(buffer[i].scale, res.scale, nw);
		}

		wSum += weight;
	}

	void Blender::Blend(ArrayView<Asset::Animation> clips) {
		if(items.empty()) {
			return;
		}

		wSum = 0.0f;
		for(size_t stateI = 0; stateI < items.size(); ++stateI) {
			BlendState(stateI, clips);
		}
	}

	MutableArrayView<BoneTransform> Blender::GetBoneTransforms()
	{
		return MutableArrayView<BoneTransform>(buffer, 128);
	}

	void Blender::UpdateMatrices(ArrayView<Asset::Bone> bones,
		Netcode::Float4x4 * toRootMatrices,
		Netcode::Float4x4 * bindMatrices)
	{
		Netcode::Matrix bindTrans;
		Netcode::Matrix toRoot[128];

		int parentId;
		for(size_t i = 0; i < bones.Size(); ++i) {
			toRoot[i] = Netcode::AffineTransformation(buffer[i].scale, buffer[i].rotation, buffer[i].translation);

			parentId = bones[i].parentId;
			if(parentId > -1) {
				// foreach parent A matrix * parent A matrix
				toRoot[i] = toRoot[i] * toRoot[bones[i].parentId];
			}
			toRootMatrices[i] = toRoot[i].Transpose();

			bindMatrices[i] = bindTrans * bones[i].transform;
		}
	}
}
