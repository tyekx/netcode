#pragma once

#include "BoneTransform.h"

#include <NetcodeFoundation/ArrayView.hpp>
#include <NetcodeAssetLib/Bone.h>

#include <algorithm>
#include <vector>
#include <cstdint>
#include <DirectXMath.h>

namespace Netcode::Animation {

	struct IKEffector {
		int32_t parentId;
		uint32_t chainLength;
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 offset;
		std::vector<BoneAngularLimit> limits;
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

		static DirectX::XMVECTOR GetP_e(const IKEffector & eff, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
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
				for(int32_t i = 0; i < numLinks; ++i) {
					// current effector "world" position
					DirectX::XMVECTOR Pe = GetP_e(effector, bones, boneTransforms);
					// link "world" position at index i
					DirectX::XMVECTOR Pc = GetP_c(effector, static_cast<uint32_t>(i), bones, boneTransforms);
					// effector target "world" position
					DirectX::XMVECTOR Pt = target;

					DirectX::XMVECTOR diffLenV = DirectX::XMVector4Length(DirectX::XMVectorSubtract(Pt, Pe));
					float diffLen;
					DirectX::XMStoreFloat(&diffLen, diffLenV);

					if(diffLen < 0.5f) {
						return;
					}

					DirectX::XMVECTOR ecDiffLengthV = DirectX::XMVector3Length(DirectX::XMVectorSubtract(Pc, Pe));
					float ecDiffLength;
					DirectX::XMStoreFloat(&ecDiffLength, ecDiffLengthV);

					if(ecDiffLength < 0.001f) {
						continue;
					}

					DirectX::XMVECTOR u = DirectX::XMVectorSubtract(Pe, Pc);
					DirectX::XMVECTOR v = DirectX::XMVectorSubtract(Pt, Pc);

					u = DirectX::XMVector3Normalize(u);
					v = DirectX::XMVector3Normalize(v);

					DirectX::XMVECTOR cosThetaV = DirectX::XMVector3Dot(u, v);
					float cosTheta;
					DirectX::XMStoreFloat(&cosTheta, cosThetaV);
					float theta = acosf(std::clamp(cosTheta, -1.0f, 1.0f));

					DirectX::XMVECTOR rotationAxis = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(u, v));

					boneTransforms[boneId].rotation =
						DirectX::XMQuaternionMultiply(boneTransforms[boneId].rotation,
							DirectX::XMQuaternionRotationAxis(rotationAxis, theta));

					if(!effector.limits.empty()) {
						DirectX::XMVECTOR quatAxis;
						float quatAngle;
						DirectX::XMQuaternionToAxisAngle(&quatAxis, &quatAngle, boneTransforms[boneId].rotation);
						float angularLimit = effector.limits[i].angleLimitInRadians;
						quatAngle = std::min(quatAngle, angularLimit);
						boneTransforms[boneId].rotation = DirectX::XMQuaternionRotationAxis(quatAxis, quatAngle);
					}


					boneId = bones[boneId].parentId;
				}
			}
		}
	};

	/*
	Forward And Backward Reaching IK
	*/
	class FABRIK {
	public:
		static void Run(IKEffector effector, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms, int32_t maxIterations = 10) {
			std::vector<BoneTransform> wt;
			std::vector<float> d;
			float sumLen = 0.0f;

			int32_t boneId = effector.parentId;
			for(uint32_t i = 0; i < effector.chainLength; ++i) {
				wt.push_back(BackwardBounceCCD::GetWorldRT(boneId, bones, boneTransforms));
				float len;
				DirectX::XMStoreFloat(&len, DirectX::XMVector3Length(boneTransforms[boneId].translation));
				sumLen += len;
				d.push_back(len);
				boneId = bones[boneId].parentId;
			}

			DirectX::FXMVECTOR t = DirectX::XMLoadFloat4(&effector.position);

			float dist;
			DirectX::XMStoreFloat(&dist, DirectX::XMVector3Length(DirectX::XMVectorSubtract(wt.back().translation, t)));
			
			// target is unreachable
			if(dist > sumLen) {
				for(uint32_t i = 1; i < effector.chainLength; ++i) {
					float ri;
					DirectX::XMStoreFloat(&ri, DirectX::XMVector3Length(DirectX::XMVectorSubtract(t, wt[i].translation)));
					float lambdai = d[i] / ri;
					DirectX::XMVECTOR pi1 = DirectX::XMVectorAdd(DirectX::XMVectorScale(wt[i].translation, 1.0f - lambdai),
						DirectX::XMVectorScale(t, lambdai));

					wt[i - 1].translation = pi1;
				}
			}
			
			wt.push_back(boneTransforms[0]);

			for(int32_t i = 4; i >= 0; --i) {
				//wt[i].translation
			}

			int32_t done;
		}
	};


}
