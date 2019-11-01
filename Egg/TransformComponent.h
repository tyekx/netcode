#pragma once

#include <DirectXMath.h>

namespace Egg {

	struct Transform {
		DirectX::XMFLOAT4 Position;
		DirectX::XMFLOAT4 Rotation;
		DirectX::XMFLOAT3 Scale;

		Transform() : Position{ 0.0f, 0.0f, 0.0f, 1.0f }, Rotation{ 0.0f, 0.0f, 0.0f, 1.0f }, Scale{ 1.0f, 1.0f, 1.0f } {

		}
	};

}