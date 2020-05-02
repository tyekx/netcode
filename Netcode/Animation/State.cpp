#include "State.h"


namespace Netcode::Animation {
	bool StateBase::IsComplete() const {
		return time > duration;
	}
	void StateBase::Clear() {
		time = 0.0f;
	}
	void StateBase::Update(float dt) {
		time += speed * ticksPerSecond * dt;

		if(time > duration) {
			if(behaviour == StateBehaviour::LOOP) {
				time = 0.0f;
			}

			if(behaviour == StateBehaviour::ONCE) {
				time = duration;
			}
		}
	}
}


