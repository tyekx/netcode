#pragma once

#include <Netcode/Modules.h>
#include <map>
#include <string>
#include <Netcode/Vertex.h>
#include <Netcode/Utility.h>
#include "GameObject.h"
#include "Mesh.h"

using Netcode::Graphics::ResourceType;
using Netcode::Graphics::ResourceState;

class AnimationController {
public:
	constexpr static uint32_t MAX_ACTIVE_STATE_COUNT = 8;
private:
	// states
	// transitions
	// active states
public:
	void Update(float dt) {}
};

class AnimationSet {
public:
	constexpr static uint32_t MAX_INSTANCE_COUNT = 32;
	constexpr static uint32_t MAX_ANIMATION_COUNT = 64;
private:

	std::vector<std::unique_ptr<AnimationController>> content;

public:
	std::unique_ptr<AnimationController> CreateController() {
		return nullptr;
	}
};

struct AnimComponent {
	std::shared_ptr<AnimationSet> animationSet;
	std::unique_ptr<AnimationController> controller;
};
