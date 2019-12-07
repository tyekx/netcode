#pragma once

#include "AnimationComponent.h"
#include "ScriptComponent.h"
#include "TransformComponent.h"
#include "PhysxComponent.h"
#include <tuple>

using COMPONENTS_T = std::tuple<Egg::Transform, ScriptComponent, PhysxComponent, AnimationComponent>;

//constexpr int ofs = TupleOffsetOf<MeshComponent, COMPONENTS_T>::value;
