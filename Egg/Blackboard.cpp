#include "Blackboard.h"


namespace Egg::Animation {

	AnimationState * Blackboard::FindReferenceByName(const std::string & name) {

		for(UINT i = 0; i < statesLength; ++i) {
			if(strcmp((states + i)->name, name.c_str()) == 0) {
				return states + i;
			}
		}

		return nullptr;
	}

	void Blackboard::CheckCurrentTransitions(MovementController * movCtrl) {

		for(UINT i = 0; i < currentState->transitionsLength; ++i) {
			AnimationState::Transition * t = currentState->transitions + i;

			if((*t)(movCtrl, currentState)) {
				switch(t->behaviour) {
				case TransitionBehaviour::STOP_AND_LERP:
					currentState->animSpeed = 0.0f;
					break;
				}
				currentState = t->targetState;
				currentState->animTime = 0.0f;
				currentState->animSpeed = 1.0f;
				break;
			}
		}
	}

	void Blackboard::Bind(BoneDataCb * writeDest) {
		dest = writeDest;
	}

	Blackboard::Blackboard() : allocator{}, prevState{ nullptr }, currentState{ nullptr }, bones{ nullptr }, dest{ nullptr }, states{ nullptr }, statesLength{ 0 } { }

	void Blackboard::CreateResources(Asset::Model * model, unsigned int animationsLength,
						const std::initializer_list<AnimationState> & sts,
						const std::initializer_list<TransitionInit> &transitions) {

		bones = model->bones;

		UINT requiredSize = sts.size() * sizeof(AnimationState) + transitions.size() * sizeof(AnimationState::Transition);
		allocator.Initialize(requiredSize);


		// step1: allocate and initialize states
		states = static_cast<AnimationState *>(allocator.Allocate(sizeof(AnimationState) * sts.size()));
		statesLength = static_cast<UINT>(sts.size());

		UINT idx = 0;
		for(auto & i : sts) {
			new (states + idx) AnimationState{ i };
			(states + idx)->SetAnimationRef(model->animations + i.id);
			++idx;
		}

		// step2: calculate the number of transitions for every state
		for(auto & t : transitions) {
			// omit nullptr check for crude error reporting
			FindReferenceByName(t.ownerState)->transitionsLength++;
		}

		// step3: allocate memory for transitions and initialize it
		for(UINT i = 0; i < statesLength; ++i) {
			AnimationState * state = states + i;
			state->transitions = static_cast<AnimationState::Transition *>(allocator.Allocate(sizeof(AnimationState::Transition) * state->transitionsLength));
			for(UINT j = 0; j < state->transitionsLength; ++j) {
				new (state->transitions + j) AnimationState::Transition{};
			}
		}

		// step4: fill transitions with meaningful data, bit wasteful but gets the job done
		for(UINT i = 0; i < statesLength; ++i) {
			AnimationState * state = states + i;
			UINT j = 0;
			for(const auto & t : transitions) {
				AnimationState* tOwner = FindReferenceByName(t.ownerState);

				if(tOwner == state) {
					AnimationState * targetState = FindReferenceByName(t.targetState);
					AnimationState::Transition * transition = state->transitions + j;
					new (state->transitions + j) AnimationState::Transition{ t.movQueryFunction, t.animQueryFunction, targetState, t.behaviour };
					++j;
				}
			}
		}

		// step5: set current transition
		currentState = states;
		currentState->animTime = 0.0f;
		currentState->animSpeed = 1.0f;
	}

	void Blackboard::Update(float dt, MovementController * movCtrl) {
		currentState->Update(dt);
		CheckCurrentTransitions(movCtrl);

		Egg::Asset::Animation * a = currentState->animationRef;
		float t = currentState->animTime;
		UINT idx;
		for(idx = 1; idx < a->keysLength; ++idx) {
			if(a->times[idx - 1] <= t && a->times[idx] >= t) {
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

}
