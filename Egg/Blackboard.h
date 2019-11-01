#pragma once

#include "AnimationState.h"
#include "Transition.h"
#include "Asset/Model.h"
#include <vector>
#include "ConstantBufferTypes.h"
#include "LinearAllocator.h"

namespace Egg::Animation {

	class Blackboard {
		Memory::LinearAllocator allocator;

		AnimationState * prevState;
		AnimationState * currentState;

		Asset::Bone * bones;
		BoneDataCb * dest;

		AnimationState * states;
		UINT statesLength;


		AnimationState * FindReferenceByName(const std::string & name);

		void CheckCurrentTransitions(MovementController * movCtrl);

	public:
		void Bind(BoneDataCb * writeDest);

		Blackboard();

		void CreateResources(Asset::Model * model, unsigned int animationsLength,
							 const std::initializer_list<AnimationState> & states,
							 const std::initializer_list<TransitionInit> & transitions);

		void Update(float dt, MovementController * movCtrl);
	};

}
