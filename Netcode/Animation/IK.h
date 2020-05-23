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
		Float4 position;
		Float4 offset;
		std::vector<BoneAngularLimit> limits;
	};

	class BackwardBounceCCD {
	public:
		static BoneTransform GetWorldRT(int32_t boneId, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
			BoneTransform bt;
			bt.scale = Float3{ 1.0f, 1.0f, 1.0f };

			if(boneId == -1) {
				return bt;
			}

			BoneTransform parent = GetWorldRT(bones[boneId].parentId, bones, boneTransforms);

			Netcode::Vector3 wPos = boneTransforms[boneId].translation.Rotate(parent.rotation) + parent.translation;
			Netcode::Quaternion wRot = boneTransforms[boneId].rotation * parent.rotation;

			bt.translation = wPos;
			bt.rotation = wRot;
			return bt;
		}

		static Netcode::Vector3 GetP_e(const IKEffector & eff, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
			Netcode::Vector3 offset = eff.offset;

			auto wrt = GetWorldRT(eff.parentId, bones, boneTransforms);

			return wrt.translation + offset;
		}

		static Netcode::Vector3  GetP_c(const IKEffector & eff, uint32_t skip, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTransforms) {
			int32_t boneId = eff.parentId;

			for(uint32_t i = 0; i < skip; ++i) {
				boneId = bones[boneId].parentId;
			}

			auto wrt = GetWorldRT(boneId, bones, boneTransforms);

			return wrt.translation;
		}

		static void Run(IKEffector effector, ArrayView<Asset::Bone> bones, MutableArrayView<BoneTransform> boneTransforms, int32_t maxIterations = 10) {
			Netcode::Vector3 target = effector.position;

			int32_t numLinks = static_cast<int32_t>(effector.chainLength);
			for(int32_t k = 0; k < maxIterations; ++k) {
				int32_t boneId = effector.parentId;
				for(int32_t i = 0; i < numLinks; ++i) {
					// current effector "world" position
					Netcode::Vector3 Pe = GetP_e(effector, bones, boneTransforms);
					// link "world" position at index i
					Netcode::Vector3 Pc = GetP_c(effector, static_cast<uint32_t>(i), bones, boneTransforms);
					// effector target "world" position
					Netcode::Vector3 Pt = target;

					float diffLen = (Pt - Pe).Length();

					if(diffLen < 0.5f) {
						return;
					}

					float ecDiffLen = (Pc - Pe).Length();

					if(ecDiffLen < 0.001f) {
						continue;
					}

					Netcode::Vector3 u = (Pe - Pc).Normalize();
					Netcode::Vector3 v = (Pt - Pc).Normalize();

					float cosTheta = u.Dot(v);
					float theta = acosf(std::clamp(cosTheta, -1.0f, 1.0f));

					Netcode::Vector3 rotationAxis = u.Cross(v).Normalize();

					boneTransforms[boneId].rotation = boneTransforms[boneId].rotation * Quaternion{ rotationAxis, theta };

					if(!effector.limits.empty()) {/*
						DirectX::XMVECTOR quatAxis;
						float quatAngle;
						DirectX::XMQuaternionToAxisAngle(&quatAxis, &quatAngle, boneTransforms[boneId].rotation);
						float angularLimit = effector.limits[i].angleLimitInRadians;
						quatAngle = std::min(quatAngle, angularLimit);
						boneTransforms[boneId].rotation = DirectX::XMQuaternionRotationAxis(quatAxis, quatAngle);*/
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
		static void Run(IKEffector effector, ArrayView<Asset::Bone> bones, MutableArrayView<BoneTransform> boneTransforms, int32_t maxIterations = 10) {
			std::vector<BoneTransform> wt;
			std::vector<float> d;
			float sumLen = 0.0f;

			int32_t boneId = effector.parentId;
			for(uint32_t i = 0; i < effector.chainLength; ++i) {
				wt.push_back(BackwardBounceCCD::GetWorldRT(boneId, bones, boneTransforms));
				float len = boneTransforms[boneId].translation.Length();
				sumLen += len;
				d.push_back(len);
				boneId = bones[boneId].parentId;
			}

			Netcode::Vector3 t = effector.position;

			float dist = (wt.back().translation - t).Length();
			
			// target is unreachable
			if(dist > sumLen) {
				for(uint32_t i = 1; i < effector.chainLength; ++i) {
					float ri = (t - wt[i].translation).Length();
					float lambdai = d[i] / ri;

					Vector3 pi1 = wt[i].translation * (1.0f - lambdai) + t * lambdai;

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
