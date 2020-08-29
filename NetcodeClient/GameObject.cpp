#include "GameObject.h"

void Script::Setup(GameObject * owner) {
	if(behavior) {
		behavior->Setup(owner);
	}
}

void Script::SetBehavior(std::unique_ptr<IBehavior> behav)
{
	behavior = std::move(behav);
}

void Script::Update(float dt)
{
	if(behavior) {
		behavior->Update(dt);
	}
}

NatvisComponentObject instanceForPDB;
NatvisExtComponentObject instanceForPDB2;
