#pragma once
#include <memory>
#include <functional>
#include <vector>
#include "AnimationController.h"

class AnimationSet : public std::enable_shared_from_this<AnimationSet> {
public:
	constexpr static uint32_t MAX_INSTANCE_COUNT = 32;
	constexpr static uint32_t MAX_ANIMATION_COUNT = 64;
private:
	uint32_t numActiveControllers;

	std::vector<std::unique_ptr<AnimationController>> freedControllers;

	void FreeController(AnimationController * rawPtr) {
		std::unique_ptr<AnimationController> wrappedPtr{ rawPtr };
		freedControllers.emplace_back(std::move(wrappedPtr));
	}

	std::shared_ptr<AnimationController> MakeNewController() {
		if(numActiveControllers >= MAX_INSTANCE_COUNT) {
			return nullptr;
		}
		++numActiveControllers;

		return std::shared_ptr<AnimationController>(
			new AnimationController(shared_from_this()),
			std::bind(&AnimationSet::FreeController, this, std::placeholders::_1));
	}

	std::shared_ptr<AnimationController> ReuseController() {
		std::unique_ptr<AnimationController> ctrl = std::move(freedControllers.back());
		freedControllers.pop_back();
		return std::shared_ptr<AnimationController>(std::move(ctrl));
	}

public:
	std::shared_ptr<AnimationController> CreateController() {
		if(freedControllers.empty()) {
			return MakeNewController();
		} else {
			return ReuseController();
		}
	}
};
