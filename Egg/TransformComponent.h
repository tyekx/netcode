#pragma once

#include "Math/Math.h"

using Egg::Math::Float4;
using Egg::Math::Float3;

struct TransformComponent {
	Float4 Position;
	Float4 Rotation;
	Float3 Scale;

	TransformComponent() : Position{ Float4::UnitW }, Rotation{ Float4::UnitW }, Scale{ Float3::One } { }
};
