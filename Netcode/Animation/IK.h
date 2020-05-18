#pragma once

#include "BoneTransform.h"

#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeAssetLib/Bone.h>

#include <vector>
#include <cstdint>
#include <DirectXMath.h>

namespace Netcode::Animation {

	struct IKEffector {
		int32_t parentId;
		uint32_t chainLength;
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 offset;
	};

	class BackwardBounceCCD {
	public:
		static BoneTransform GetWorldRT(int32_t boneId, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
			if(boneId == -1) {
				BoneTransform bt;
				bt.translation = DirectX::XMQuaternionIdentity();
				bt.scale = DirectX::XMVectorReplicate(1.0f);
				bt.rotation = DirectX::XMQuaternionIdentity();
				return bt;
			}

			BoneTransform parent = GetWorldRT(bones[boneId].parentId, bones, boneTransforms);

			DirectX::XMVECTOR wPos = DirectX::XMVectorAdd(DirectX::XMVector3Rotate(boneTransforms[boneId].translation, parent.rotation), parent.translation);
			DirectX::XMVECTOR wRot = DirectX::XMQuaternionMultiply(boneTransforms[boneId].rotation, parent.rotation);
		
			BoneTransform bt;
			bt.scale = DirectX::XMVectorReplicate(1.0f);
			bt.translation = wPos;
			bt.rotation = wRot;
			return bt;
		}

		static DirectX::XMMATRIX GetToRoot(uint32_t boneId, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
			DirectX::XMMATRIX mats = DirectX::XMMatrixIdentity();

			int32_t bid = static_cast<int32_t>(boneId);

			while(bid >= 0) {
				DirectX::XMMATRIX anim = DirectX::XMMatrixAffineTransformation(boneTransforms[boneId].scale,
					DirectX::XMQuaternionIdentity(),
					boneTransforms[boneId].rotation,
					boneTransforms[boneId].translation);
				mats = DirectX::XMMatrixMultiply(mats, anim);
				bid = bones[bid].parentId;
			}

			return mats;
		}

		static DirectX::XMVECTOR GetP_e(const IKEffector& eff, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
			DirectX::XMVECTOR offset = DirectX::XMLoadFloat4(&eff.offset);

			auto wrt = GetWorldRT(eff.parentId, bones, boneTransforms);

			return DirectX::XMVectorAdd(wrt.translation, offset);
		}

		static DirectX::XMVECTOR GetP_c(const IKEffector & eff, uint32_t skip, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
			int32_t boneId = eff.parentId;
			
			for(uint32_t i = 0; i < skip; ++i) {
				boneId = bones[boneId].parentId;
			}

			auto wrt = GetWorldRT(boneId, bones, boneTransforms);

			return wrt.translation;
		}

		static void Run(IKEffector effector, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms, int32_t maxIterations = 10) {
			DirectX::XMVECTOR target = DirectX::XMLoadFloat4(&effector.position);

			int32_t numLinks = static_cast<int32_t>(effector.chainLength);

			for(int32_t k = 0; k < maxIterations; ++k) {

				int32_t boneId = effector.parentId;
				for(int32_t i =  0; i < numLinks; ++i) {
					// current effector "world" position
					DirectX::XMVECTOR Pe = GetP_e(effector, bones, boneTransforms);
					// link "world" position at index i
					DirectX::XMVECTOR Pc = GetP_c(effector, static_cast<uint32_t>(i), bones, boneTransforms);
					// effector target "world" position
					DirectX::XMVECTOR Pt = target;

					DirectX::XMVECTOR ecDiffLengthV = DirectX::XMVector3Length(DirectX::XMVectorSubtract(Pc, Pe));
					float ecDiffLength;
					DirectX::XMStoreFloat(&ecDiffLength, ecDiffLengthV);

					if(ecDiffLength < 0.001f) {
						continue;
					}

					DirectX::XMVECTOR diffLenV = DirectX::XMVector4Length(DirectX::XMVectorSubtract(Pt, Pe));
					float diffLen;
					DirectX::XMStoreFloat(&diffLen, diffLenV);

					if(diffLen < 0.5f) {
						return;
					}

					DirectX::XMVECTOR u = DirectX::XMVectorSubtract(Pe, Pc);
					DirectX::XMVECTOR v = DirectX::XMVectorSubtract(Pt, Pc);

					u = DirectX::XMVector3Normalize(u);
					v = DirectX::XMVector3Normalize(v);

					DirectX::XMVECTOR cosThetaV = DirectX::XMVector3Dot(u, v);
					float cosTheta;
					DirectX::XMStoreFloat(&cosTheta, cosThetaV);

					DirectX::XMVECTOR rotationAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v));

					boneTransforms[boneId].rotation =
						DirectX::XMQuaternionMultiply(boneTransforms[boneId].rotation,
						DirectX::XMQuaternionRotationAxis(rotationAxis, acosf(cosTheta)));

					boneId = bones[boneId].parentId;
				}
			}
		}
	};

}
