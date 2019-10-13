#pragma once

#include <PxActor.h>

enum class PxComponentState {
	UNDEFINED, UNINITIALIZED, PLAYING, WAITING_FOR_DELETION
};

struct PhysxComponent {
	PxComponentState State;
	physx::PxActor * Actor;

	

	PhysxComponent() : Actor{ nullptr }, State{ PxComponentState::UNDEFINED } { 
		//Actor = 
	}
};
