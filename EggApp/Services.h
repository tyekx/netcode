#pragma once

#include <Egg/Service.hpp>
#include "AssetManager.h"
#include "GameScene.h"
#include "UIScene.h"

using ServicesTuple = std::tuple<
	AssetManager,
	GameScene,
	UIScene
>;


using Service = Egg::Service<ServicesTuple>;
