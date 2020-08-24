#pragma once

#include <Netcode/Functions.h>
#include <NetcodeAssetLib/Model.h>
#include <json11.hpp>

#include "GameObject.h"

void CreateYbotAnimationComponent(Netcode::Asset::Model * model, Animation * anim);

json11::Json LoadJsonFile(const std::wstring & absolutePath);
