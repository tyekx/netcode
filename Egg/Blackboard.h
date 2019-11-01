#pragma once

#include "AnimationState.h"
#include "Transition.h"
#include "Asset/Model.h"
#include <vector>
#include "ConstantBufferTypes.h"
#include "LinearAllocator.h"
#include "AnimationBlender.h"

namespace Egg::Animation {



	class Blackboard {
		Memory::LinearAllocator allocator;

		AnimationState * prevState;
		AnimationState * currentState;

		AnimationState * states;
		UINT statesLength;

		AnimationBlender * blender;


		AnimationState * FindReferenceByName(const std::string & name);

		void CheckCurrentTransitions(MovementController * movCtrl);

	public:

		Blackboard();

		void CreateResources(Asset::Model * model, BoneDataCb * writeDest, unsigned int animationsLength,
							 const std::initializer_list<AnimationState> & states,
							 const std::initializer_list<TransitionInit> & transitions);

		void Update(float dt, MovementController * movCtrl);
	};

}
