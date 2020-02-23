#pragma once

#include <Egg/Service.hpp>
#include "AssetManager.h"
#include "GameScene.h"

using ServicesTuple = std::tuple<
	AssetManager,
	GameScene
>;


using Service = Egg::Service<ServicesTuple>;
