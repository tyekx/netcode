#pragma once

#include "ScriptComponent.h"
#include "TransformComponent.h"
#include "MeshComponent.h"
#include <tuple>

using COMPONENTS_T = std::tuple<TransformComponent, ScriptComponent, MeshComponent>;

//constexpr int ofs = TupleOffsetOf<MeshComponent, COMPONENTS_T>::value;
