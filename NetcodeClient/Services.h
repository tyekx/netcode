#pragma once

#include <Netcode/Service.hpp>
#include "AssetManager.h"
#include "GameScene.h"
#include <Netcode/PhysXWrapper.h>

using ServicesTuple = std::tuple<AssetManager, GameSceneManager, Netcode::Physics::PhysX, Netcode::Module::IGraphicsModule*>;

using Service = Netcode::Service<ServicesTuple>;
