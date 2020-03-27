#include "AnimationBlender.h"
#include <memory>

namespace Netcode::Animation {


	AnimationBlender::AnimationBlender(Asset::Bone * skeleton, unsigned int skeletonLength) {
		bones = skeleton;
		bonesLength = skeletonLength;
		numActiveStates = 0;
		memset(activeStates, 0, sizeof(AnimationState *) * MAX_ACTIVE_STATE_COUNT);
	}

	void AnimationBlender::ActivateState(AnimationState * state) {
		if(numActiveStates < MAX_ACTIVE_STATE_COUNT) {
			for(unsigned int i = 0; i < numActiveStates; ++i) {
				if(activeStates[i] == state) {
					return;
				}
			}
			activeStates[numActiveStates++] = state;
		}
	}

	void AnimationBlender::Blend() {
		if(numActiveStates == 0) {
			return;
		}

		float weightSum = 0.0f;

		DirectX::XMVECTOR stPos;
		DirectX::XMVECTOR endPos;

		DirectX::XMVECTOR stQuat;
		DirectX::XMVECTOR endQuat;

		DirectX::XMVECTOR stScale;
		DirectX::XMVECTOR endScale;

		DirectX::XMMATRIX bindTrans;
		DirectX::XMMATRIX toRoot[128];
		int parentId;


		for(unsigned int ai = 0; ai < numActiveStates; ++ai) {
			weightSum += activeStates[ai]->weight;
		}

		for(unsigned int ai = 0; ai < numActiveStates; ++ai) {
			float weight = activeStates[ai]->weight;

			Netcode::Asset::Animation * a = activeStates[ai]->animationRef;
			float t = activeStates[ai]->animTime;
			unsigned int idx;
			for(idx = 1; idx < a->keysLength; ++idx) {
				if(a->times[idx - 1] <= t && a->times[idx] >= t) {
					t = (t - a->times[idx - 1]) / (a->times[idx] - a->times[idx - 1]);
					break;
				}
			}

			auto * startKey = (a->keys + (idx - 1) * a->bonesLength);
			auto * endKey = (a->keys + idx * a->bonesLength);

			for(unsigned int i = 0; i < a->bonesLength; ++i) {
				stPos = DirectX::XMLoadFloat3(&startKey[i].position);
				stQuat = DirectX::XMLoadFloat4(&startKey[i].rotation);
				stScale = DirectX::XMLoadFloat3(&startKey[i].scale);

				endPos = DirectX::XMLoadFloat3(&endKey[i].position);
				endQuat = DirectX::XMLoadFloat4(&endKey[i].rotation);
				endScale = DirectX::XMLoadFloat3(&endKey[i].scale);

				stPos = DirectX::XMVectorLerp(stPos, endPos, t);
				stQuat = DirectX::XMQuaternionSlerp(stQuat, endQuat, t);
				stScale = DirectX::XMVectorLerp(stScale, endScale, t);

				stPos = DirectX::XMVectorScale(stPos, weight / weightSum);
				stQuat = DirectX::XMQuaternionSlerp(DirectX::XMQuaternionIdentity(), stQuat, weight / weightSum);
				stScale = DirectX::XMVectorScale(stScale, weight / weightSum);

				// first item case, serves to clear the buffer
				if(ai == 0) {
					buffer[i].translation = stPos;
					buffer[i].rotation = stQuat;
					buffer[i].scale = stScale;
				} else {
					buffer[i].translation = DirectX::XMVectorAdd(buffer[i].translation, stPos);
					buffer[i].rotation = DirectX::XMQuaternionMultiply(buffer[i].rotation, stQuat);
					buffer[i].scale = DirectX::XMVectorAdd(buffer[i].scale, stScale);
				}
			}
		}

		for(unsigned int i = 0; i < bonesLength; ++i) {
			toRoot[i] = DirectX::XMMatrixAffineTransformation(buffer[i].scale, DirectX::XMQuaternionIdentity(), buffer[i].rotation, buffer[i].translation);

			parentId = bones[i].parentId;
			if(parentId > -1) {
				toRoot[i] = DirectX::XMMatrixMultiply(toRoot[i], toRoot[bones[i].parentId]);
			}

			DirectX::XMStoreFloat4x4A(toRootTransform + i, DirectX::XMMatrixTranspose(toRoot[i]));

			bindTrans = DirectX::XMLoadFloat4x4(&bones[i].transform);
			bindTrans = DirectX::XMMatrixMultiply(bindTrans, toRoot[i]);

			DirectX::XMStoreFloat4x4A(bindTransform + i, DirectX::XMMatrixTranspose(bindTrans));
		}
	}

	void AnimationBlender::UpdateStates(float dt) {
		for(unsigned int i = 0; i < numActiveStates;) {
			if(activeStates[i] == nullptr) {
				break;
			}
			activeStates[i]->weight += activeStates[i]->weightVelocity * dt;
			activeStates[i]->weight = fminf(activeStates[i]->weight, 1.0f);
			if(activeStates[i]->weight < 0.0f) {
				activeStates[i] = nullptr;
				std::swap(activeStates[i], activeStates[--numActiveStates]);
				continue;
			}
			++i;
		}
	}

	void AnimationBlender::CopyBoneDataInto(void * dest)
	{
		memcpy(dest, bindTransform, sizeof(DirectX::XMFLOAT4X4A) * bonesLength);
	}

	void AnimationBlender::CopyToRootDataInto(void * dest)
	{
		memcpy(dest, toRootTransform, sizeof(DirectX::XMFLOAT4X4A) * bonesLength);
	}

}
