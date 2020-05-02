#pragma once

#include <bitset>
#include <cstdint>
#include <vector>

#include "Transition.h"

namespace Netcode::Animation {

	using BoneMask = std::bitset<128>;

	enum class StateBehaviour : uint32_t {
		NONE, LOOP, ONCE
	};

	class StateBase {
	public:
		std::string name;
		BoneMask boneMask;
		int32_t clipId;
		StateBehaviour behaviour;
		float duration;
		float ticksPerSecond;
		float speed;
		float time;
		float weight;

		bool IsComplete() const;

		void Clear();

		void Update(float dt);

		StateBase(const std::string & name, BoneMask boneMask, StateBehaviour behaviour, int32_t clipId, float duration, float ticksPerSecond, float speed = 1.0f) :
			name{ name }, boneMask{ boneMask }, clipId{ clipId }, behaviour{ behaviour }, duration{ duration }, ticksPerSecond{ ticksPerSecond }, speed{ speed }, time{ 0.0f }, weight{ 0.0f }  {

		}
	};

	template<typename T>
	class State : public StateBase {
		std::vector<Transition<T>> transitions;
	public:
		using StateBase::StateBase;

		void SetTransitions(std::vector<Transition<T>> trs) {
			transitions = std::move(trs);
		}

		Transition<T> * TryTransition(T * transitionController) {
			for(Transition<T> & t : transitions) {
				if(t(transitionController, this)) {
					return &t;
				}
			}
			return nullptr;
		}
	};

}

