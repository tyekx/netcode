#pragma once

#include "AnimationState.h"
#include "Transition.h"
#include "Asset/Model.h"
#include <vector>

namespace Egg::Animation {

	class Blackboard {
		// storage
		std::vector<AnimationState> States;
		std::vector<Transition> Transitions;

		AnimationState * prevState;
		AnimationState * currentState;

		Asset::Bone * bones;
		BoneDataCb * dest;
	public:
		void Bind(BoneDataCb * writeDest) {
			dest = writeDest;
		}

		Blackboard() : States{}, Transitions{}, prevState{ nullptr }, currentState{ nullptr }, dest{ nullptr } { }

		void CreateResources(Asset::Model* model, unsigned int animationsLength,
				   std::initializer_list<AnimationState> states,
				   std::initializer_list<Transition> transitions) {
			 
			States = states;
			Transitions = transitions;

			bones = model->bones;


			for(auto & i : States) {
				i.SetAnimationRef(model->animations + i.id);
			}

			prevState = nullptr;
			currentState = (States.size() > 0) ? &(States.at(0)) : nullptr;
			if(currentState) {
				currentState->animSpeed = 1.0f;
				currentState->weight = 1.0f;
			}
		}

		void Update(float dt, MovementController * movCtrl) {
			currentState->Update(dt);

			for(auto & i : Transitions) {
				if(i.Owns(currentState)) {
					if(i(movCtrl, currentState)) {
						currentState = i.pTargetState;
					}
				}
			}

			Egg::Asset::Animation * a = currentState->animationRef;
			float t = currentState->animTime;
			UINT idx;
			for(idx = 1; idx < a->keysLength; ++idx) {
				if(a->times[idx - 1] < t && a->times[idx] > t) {
					t = (t - a->times[idx - 1]) / (a->times[idx] - a->times[idx - 1]);
					break;
				}
			}

			auto * startKey = (a->keys + (idx - 1) * a->bonesLength);
			auto * endKey = (a->keys + idx * a->bonesLength);

			DirectX::XMVECTOR stPos;
			DirectX::XMVECTOR endPos;

			DirectX::XMVECTOR stQuat;
			DirectX::XMVECTOR endQuat;

			DirectX::XMVECTOR stScale;
			DirectX::XMVECTOR endScale;

			DirectX::XMMATRIX toRoot[128];
			int parentId;

			DirectX::XMMATRIX bindTrans;

			for(UINT i = 0; i < a->bonesLength; ++i) {
				stPos = DirectX::XMLoadFloat3(&startKey[i].position);
				stQuat = DirectX::XMLoadFloat4(&startKey[i].rotation);
				stScale = DirectX::XMLoadFloat3(&startKey[i].scale);

				endPos = DirectX::XMLoadFloat3(&endKey[i].position);
				endQuat = DirectX::XMLoadFloat4(&endKey[i].rotation);
				endScale = DirectX::XMLoadFloat3(&endKey[i].scale);

				stPos = DirectX::XMVectorLerp(stPos, endPos, t);
				stQuat = DirectX::XMQuaternionSlerp(stQuat, endQuat, t);
				stScale = DirectX::XMVectorLerp(stScale, endScale, t);

				toRoot[i] = DirectX::XMMatrixAffineTransformation(stScale, DirectX::XMQuaternionIdentity(), stQuat, stPos);
				
				parentId = bones[i].parentId;
				if(parentId > -1) {
					toRoot[i] = DirectX::XMMatrixMultiply(toRoot[i], toRoot[bones[i].parentId]);
				}

				DirectX::XMStoreFloat4x4A(dest->ToRootTransform + i, DirectX::XMMatrixTranspose(toRoot[i]));

				bindTrans = DirectX::XMLoadFloat4x4(&bones[i].transform);
				bindTrans = DirectX::XMMatrixMultiply(bindTrans, toRoot[i]);

				DirectX::XMStoreFloat4x4A(dest->BindTransform + i, DirectX::XMMatrixTranspose(bindTrans));
			}
		}
	};

}
