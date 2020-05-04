#pragma once

#include <memory>
#include <Netcode/Animation/Blender.h>

class AnimationSet;

class AnimationController {
private:
	std::shared_ptr<AnimationSet> animationSet;

	int32_t entryId;

public:
	AnimationController(std::shared_ptr<AnimationSet> animationSet);

	int32_t GetId() const {
		return entryId;
	}

	std::shared_ptr<AnimationSet> GetAnimationSet() const {
		return animationSet;
	}

	void Animate(const std::vector<Netcode::Animation::BlendItem> & blendPlan);

	friend class AnimationSet;
};
