#include "Blender.h"

namespace Netcode::Animation {
	void Blender::FillFrameData(Asset::Animation & clip, const StateBase & state, BlendItem & item) {
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
		for(StateBase & state : activeStates) {
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

	static DirectX::XMMATRIX MyQuaternionToMatrix(DirectX::XMVECTOR qt) {
		DirectX::XMFLOAT4 q;
		DirectX::XMStoreFloat4(&q, qt);

		
		DirectX::XMFLOAT4X4 mat(
			1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z, 2.0f * q.x * q.y + 2.0f * q.z * q.w, 2.0f * q.x * q.z - 2.0f * q.y * q.w, 0.0f,
			2.0f * q.x * q.y - 2.0f * q.z * q.w, 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z, 2.0f * q.y * q.z + 2.0f * q.x * q.w, 0.0f,
			2.0f * q.x * q.z + 2.0f * q.y * q.w, 2.0f * q.y * q.z - 2.0f * q.x * q.w, 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		);

		return DirectX::XMLoadFloat4x4(&mat);
	}

	static DirectX::XMMATRIX MyScaleMat(DirectX::XMVECTOR scale) {
		DirectX::XMFLOAT4 s;
		DirectX::XMStoreFloat4(&s, scale);
		
		DirectX::XMFLOAT4X4 mat(s.x, 0.0f, 0.0f, 0.0f, 0.0f, s.y, 0.0f, 0.0f, 0.0f, 0.0f, s.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		return DirectX::XMLoadFloat4x4(&mat);
	}

	static DirectX::XMMATRIX MyTranslationMat(DirectX::XMVECTOR translation) {
		DirectX::XMFLOAT4 t;
		DirectX::XMStoreFloat4(&t, translation);

		DirectX::XMFLOAT4X4 mat(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, t.x, t.y, t.z, t.w);

		return DirectX::XMLoadFloat4x4(&mat);
	}

	static DirectX::XMVECTOR MySlerp(DirectX::XMVECTOR q0, DirectX::XMVECTOR q1, float t) {
		DirectX::XMVECTOR dwL = DirectX::XMVector4Length(DirectX::XMVectorSubtract(q0, q1));
		DirectX::XMVECTOR owL = DirectX::XMVector4Length(DirectX::XMVectorAdd(q0, q1));

		float directWayLength;
		float oppositeWayLength;
		DirectX::XMStoreFloat(&directWayLength, dwL);
		DirectX::XMStoreFloat(&oppositeWayLength, owL);

		if(directWayLength > oppositeWayLength) {
			q1 = DirectX::XMVectorNegate(q1);
		}

		DirectX::XMVECTOR cosOmegaV = DirectX::XMVector4Dot(q0, q1);

		float cosOmega;
		DirectX::XMStoreFloat(&cosOmega, cosOmegaV);

		if(cosOmega > (0.999f)) {
			return DirectX::XMVector4Normalize(DirectX::XMVectorLerp(q0, q1, t));
		}

		float omega = acos(cosOmega);
		float sinOmega = sin(omega);
		float sinTOmega = sin(t * omega);
		float sinOneMinusTOmega = sin((1.0f - t) * omega);

		return DirectX::XMVector4Normalize(DirectX::XMVectorScale(
			DirectX::XMVectorAdd(
				DirectX::XMVectorScale(q0, sinOneMinusTOmega),
				DirectX::XMVectorScale(q1, sinTOmega)), 1.0f / sinOmega));
	}

	void Blender::Blend(ArrayView<Asset::Bone> bones, ArrayView<Asset::Animation> clips, DirectX::XMFLOAT4X4A * toRootMatrices, DirectX::XMFLOAT4X4A * bindMatrices) {
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
		float wSum = 0.0f;

		int parentId;


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
				stQuat = MySlerp(stQuat, endQuat, t);
				stScale = DirectX::XMVectorLerp(stScale, endScale, t);


				float nw;
				if(wInc == 0.0f) {
					nw = 0.0f;
				} else {
					nw = weight / wInc;
				}

				buffer[i].translation = DirectX::XMVectorLerp(buffer[i].translation, stPos, nw);
				buffer[i].rotation = MySlerp(buffer[i].rotation, stQuat, nw);
				buffer[i].scale = DirectX::XMVectorLerp(buffer[i].scale, stScale, nw);
			}

			wSum = wInc;
		}

		for(size_t i = 0; i < bones.Size(); ++i) {
			// A matrix
			auto sc = MyScaleMat(buffer[i].scale);
			auto rot = MyQuaternionToMatrix(buffer[i].rotation);
			auto tr = MyTranslationMat(buffer[i].translation);

			toRoot[i] = DirectX::XMMatrixMultiply(
				DirectX::XMMatrixMultiply(sc, rot), tr
			);

			//toRoot[i] = DirectX::XMMatrixAffineTransformation(buffer[i].scale, DirectX::XMQuaternionIdentity(), buffer[i].rotation, buffer[i].translation);

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
}
