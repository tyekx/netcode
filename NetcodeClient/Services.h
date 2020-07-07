#pragma once

#include <Netcode/Service.hpp>
#include "AssetManager.h"
#include "GameScene.h"

using ServicesTuple = std::tuple<AssetManager, GameScene>;

using Service = Netcode::Service<ServicesTuple>;
