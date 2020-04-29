#pragma once

#include <memory>

class AnimationSet;

class AnimationController {
public:
	constexpr static uint32_t MAX_ACTIVE_STATE_COUNT = 8;
private:
	std::shared_ptr<AnimationSet> animationSet;
	// states
	// transitions
	// active states
public:
	AnimationController(std::shared_ptr<AnimationSet> animationSet);

	void Update(float dt) { }
};
