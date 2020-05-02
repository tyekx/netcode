#include "Transition.h"
#include <algorithm>

namespace Netcode::Animation {
	TransitionBase::TransitionBase(Function::Scalar weightFunction, Function::Scalar timeScaleFunction, float duration) :
		weightFunction{ weightFunction }, timeScaleFunction{ timeScaleFunction }, time{ 0.0f }, duration{ duration }  {
	}

	float TransitionBase::GetNormalizedTime() const {
		return std::clamp(time / duration, 0.0f, 1.0f);
	}

	float TransitionBase::GetStateTimeIncrement(float deltaTime) const {
		return timeScaleFunction(GetNormalizedTime()) * deltaTime;
	}

	float TransitionBase::GetStateWeight() const {
		return weightFunction(GetNormalizedTime());
	}

	bool TransitionBase::IsComplete() const {
		return time > duration;
	}

	void TransitionBase::Update(float dt) {
		time += dt;
	}

	void TransitionBase::Clear() {
		time = 0.0f;
	}
}

