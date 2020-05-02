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
	};

	class Blender {

		struct BoneSRT {
			DirectX::XMVECTOR translation;
			DirectX::XMVECTOR rotation;
			DirectX::XMVECTOR scale;
		};

		std::vector<BlendItem> items;
		BoneSRT buffer[128];

		void FillFrameData(Asset::Animation & clip, const StateBase & state, BlendItem & item) {
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

	public:
		void UpdatePlan(ArrayView<Asset::Animation> clips, ArrayView<StateBase> activeStates) {
			items.clear();
			for(StateBase & state : activeStates) {
				BlendItem item;
				item.clipId = static_cast<uint32_t>(state.clipId);
				item.weight = state.weight;
				FillFrameData(clips[item.clipId], state, item);
				items.push_back(item);
			}
		}

		const std::vector<BlendItem> & GetPlan() const {
			return items;
		}

		void Blend( ArrayView<Asset::Bone> bones,
					ArrayView<Asset::Animation> clips,
					DirectX::XMFLOAT4X4A * toRootMatrices,
					DirectX::XMFLOAT4X4A * bindMatrices) {
			if(items.empty()) {
				return;
			}

			DirectX::XMVECTOR stPos;
			DirectX::XMVECTOR endPos;

			DirectX::XMVECTOR stQuat;
			DirectX::XMVECTOR endQuat;

			DirectX::XMVECTOR stScale;
			DirectX::XMVECTOR endScale;

			DirectX::XMMATRIX bindTrans;
			DirectX::XMMATRIX toRoot[128];
			int parentId;

			float wSum = 0.0f;

			for(const BlendItem & item : items) {
				float weight = item.weight;
				float t = item.normalizedTime;
				const Asset::Animation * a = clips.Data() + item.clipId;
				uint32_t idx0 = item.beginFrameIndex;
				uint32_t idx1 = item.endFrameIndex;

				auto * startKey = (a->keys + idx0 * a->bonesLength);
				auto * endKey = (a->keys + idx1 * a->bonesLength);

				const float wInc = wSum + weight;

				for(unsigned int i = 0; i < a->bonesLength; ++i) {
					stPos = DirectX::XMLoadFloat4(&startKey[i].position);
					stQuat = DirectX::XMLoadFloat4(&startKey[i].rotation);
					stScale = DirectX::XMLoadFloat4(&startKey[i].scale);

					endPos = DirectX::XMLoadFloat4(&endKey[i].position);
					endQuat = DirectX::XMLoadFloat4(&endKey[i].rotation);
					endScale = DirectX::XMLoadFloat4(&endKey[i].scale);

					stPos = DirectX::XMVectorLerp(stPos, endPos, t);
					stQuat = DirectX::XMQuaternionSlerp(stQuat, endQuat, t);
					stScale = DirectX::XMVectorLerp(stScale, endScale, t);


					float nw;
					if(wInc == 0.0f) {
						nw = 0.0f;
					} else {
						nw = weight / wInc;
					}

					buffer[i].translation = DirectX::XMVectorLerp(buffer[i].translation, stPos, nw);
					buffer[i].rotation = DirectX::XMQuaternionSlerp(buffer[i].rotation, stQuat, nw);
					buffer[i].scale = DirectX::XMVectorLerp(buffer[i].scale, stScale, nw);
				}

				wSum = wInc;
			}

			for(size_t i = 0; i < bones.Size(); ++i) {
				// A matrix
				toRoot[i] = DirectX::XMMatrixAffineTransformation(buffer[i].scale, DirectX::XMQuaternionIdentity(), buffer[i].rotation, buffer[i].translation);

				parentId = bones[i].parentId;
				if(parentId > -1) {
					// foreach parent A matrix * parent A matrix
					toRoot[i] = DirectX::XMMatrixMultiply(toRoot[i], toRoot[bones[i].parentId]);
				}


				DirectX::XMStoreFloat4x4A(toRootMatrices + i, DirectX::XMMatrixTranspose(toRoot[i]));

				bindTrans = DirectX::XMLoadFloat4x4(&bones[i].transform);
				bindTrans = DirectX::XMMatrixMultiply(bindTrans, toRoot[i]);

				DirectX::XMStoreFloat4x4A(bindMatrices + i, DirectX::XMMatrixTranspose(bindTrans));
			}
		}
	};

}
