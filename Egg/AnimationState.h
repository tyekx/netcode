#pragma once

#include <string>
#include "MovementController.h"
#include "Asset/Animation.h"

namespace Egg {

	enum class StateBehaviour {
		NONE, LOOP, ONCE
	};

	class AnimationState {
	public:
		std::string name;
		StateBehaviour behaviour;
		unsigned int id;
		float weight;
		float animTime;
		float animSpeed;
		float animTicksPerSecond;
		float animDuration;
		Asset::Animation * animationRef;

		void SetAnimationRef(Asset::Animation * aRef) {
			animationRef = aRef;
			animDuration = (float)aRef->duration;
			animTicksPerSecond = (float)aRef->ticksPerSecond;
		}

		AnimationState(const std::string & name, unsigned int animId, StateBehaviour behaviour) :
			name{ name }, behaviour{ behaviour }, id{ animId }, weight{ 0.0f }, animTime{ 0.0f }, animSpeed{ 0.0f }, animTicksPerSecond{ 0.0f }, animDuration{ 0.0f }, animationRef{ nullptr } { }

		void Update(float dt) {
			animTime += animSpeed * (animTicksPerSecond * dt);

			switch(behaviour) {
			case StateBehaviour::LOOP:
				animTime = fmodf(animTime, animDuration);
				break;
			case StateBehaviour::ONCE:
				animTime = fminf(animTime, animDuration);
				break;
			default:
				break;
			}
		}

		bool IsFinished() const {
			return false;
		}

		bool IsPlaying() const {
			return false;
		}
	};

}
