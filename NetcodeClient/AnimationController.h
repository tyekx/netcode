#pragma once

#include <Netcode/HandleDecl.h>
#include <memory>
#include <Netcode/Animation/Blender.h>

class AnimationSet;

class AnimationController {
private:
	Ref<AnimationSet> animationSet;

	int32_t entryId;

public:
	AnimationController(Ref<AnimationSet> animationSet);

	int32_t GetId() const {
		return entryId;
	}

	Ref<AnimationSet> GetAnimationSet() const {
		return animationSet;
	}

	void Update(const std::vector<Netcode::Animation::BlendItem> & blendPlan);

	friend class AnimationSet;
};
