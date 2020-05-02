#pragma once

#include <memory>

#include <NetcodeFoundation/ArrayView.hpp>

#include "StateGroup.h"

namespace Netcode::Animation {


	class BlackboardBase {
	public:
		virtual ~BlackboardBase() = default;

		virtual void BindController(void * pController) = 0;
		virtual void Update(float dt) = 0;
		virtual ArrayView<StateBase> GetActiveStates() = 0;
	};

	template<typename T>
	class Blackboard : public BlackboardBase {
		T * controllerReference;
		std::vector<StateGroup<T>> groups;
		std::vector<StateBase> activeStates;
	public:

		Blackboard(std::initializer_list<StateGroup<T>> groups) : controllerReference{ nullptr }, groups{ groups }, activeStates{ } {

		}

		virtual void BindController(void * pController) override {
			controllerReference = (reinterpret_cast<T *>(pController));
			for(StateGroup<T> & i : groups) {
				i.SetDefaults();
			}
		}

		virtual void Update(float dt) override {
			activeStates.clear();
			for(StateGroup<T> & i : groups) {
				i.Update(controllerReference, dt);
				i.GetActiveStates(activeStates);
			}
		}

		virtual ArrayView<StateBase> GetActiveStates() override {
			return ArrayView<StateBase>(activeStates.data(), activeStates.size());
		}
	};

}
