#include "IK.h"
#include <numeric>
#include <Netcode/Logger.h>

namespace Netcode::Animation {

	struct FABRIKLink {
		Vector3 position;
		Vector3 startPosition;
		int boneId;
	};

	static Matrix GetBoneMatrix(const BoneTransform & bt) {
		return AffineTransformation(bt.scale, bt.rotation, bt.translation);
	}

	static Matrix GetBoneChainMatrix(int boneId, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTr) {
		Matrix m;
		do {
			int parentId = bones[boneId].parentId;
			m = AffineTransformation(boneTr[boneId].scale, boneTr[boneId].rotation, boneTr[boneId].translation) * m;
			boneId = parentId;
		} while(boneId != -1);
		return m;
	}

	static void CreateLinks(std::vector<FABRIKLink> & links, ArrayView<Asset::Bone> bones, ArrayView<BoneTransform> boneTr, int boneId, uint32_t chainLength) {
		if(chainLength == 0) {
			return;
		}
		int parentId = bones[boneId].parentId;

		CreateLinks(links, bones, boneTr, parentId, chainLength - 1);

		Matrix parentM = GetBoneChainMatrix(parentId, bones, boneTr);

		FABRIKLink link;
		link.position = Vector4{ Float4::UnitW }.Transform(parentM).XYZ();
		link.startPosition = link.position;
		link.boneId = boneId;
		links.push_back(link);
	}
	
	void FABRIK::Run(IKEffector effector, ArrayView<Asset::Bone> bones, MutableArrayView<BoneTransform> boneTransforms, int32_t maxIterations) {
		std::vector<FABRIKLink> wt;
		std::vector<float> d;
		
		CreateLinks(wt, bones, boneTransforms, effector.parentId, effector.chainLength);

		for(int i = 0; i < (int)effector.chainLength - 1; i++) {
			d.push_back((wt[i + 1].startPosition - wt[i].startPosition).Length());
		}

		float sumLen = std::accumulate(std::begin(d), std::end(d), 0.0f);
		
		Vector3 target = Vector3{ effector.position };

		float dist = (wt.front().position - target).Length();

		// target is unreachable
		if(dist > sumLen) {
			for(uint32_t i = 0; i < effector.chainLength-1; ++i) {
				float ri = (target - wt[i].position).Length();
				float lambdai = d[i] / ri;
				wt[i + 1].position = (1.0f - lambdai) * wt[i].position + lambdai * target;
			}
		} else {
			Vector3 b = wt.front().position;

			float diffA = (wt.back().position - target).LengthSq();

			int32_t iter = 0;
			while(diffA > 0.1f) {
				iter++;

				if(maxIterations < iter) {
					Log::Debug("Max iterations reached");
				}
				
				// forward
				wt.back().position = target;
				for(int i = (int)wt.size() - 2; i >= 0; i--) {
					float ri = (wt[i + 1].position - wt[i].position).Length();
					float lambdai = d[i] / ri;
					wt[i].position = (1.0f - lambdai) * wt[i + 1].position + lambdai * wt[i].position;
				}

				// backward
				wt.front().position = b;
				for(int i = 0; i < (int)effector.chainLength - 1; i++) {
					float ri = (wt[i + 1].position - wt[i].position).Length();
					float lambdai = d[i] / ri;
					wt[i + 1].position = (1.0f - lambdai) * wt[i].position + lambdai * wt[i + 1].position;
				}

				diffA = (wt.back().position - target).LengthSq();
			}
		}
		
		int k = 0;
		for(int i = 1; i < effector.chainLength; i++) {
			int boneId = wt[i].boneId;
			int parentBoneId = wt[i - 1].boneId;
			
			Vector3 p0 = (wt[i].startPosition - wt[i-1].position);
			Vector3 p1 = (wt[i].position - wt[i - 1].position);
			
			float lsq0 = p0.LengthSq();
			float lsq1 = p1.LengthSq();

			Quaternion rot = Quaternion::FromRotationBetween(p0, p1, Float3::Zero);

			Vector3 chk = p0.Rotate(rot);
			
			boneTransforms[boneId].rotation = boneTransforms[boneId].rotation * rot;
			
			k++;
		}
	}

}
