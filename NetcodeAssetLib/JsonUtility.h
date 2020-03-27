#pragma once

#include <json11/json11.hpp>
#include <DirectXMath.h>
#include "Collider.h"

/*
Helper functions for primitive type serialization
*/

namespace Netcode::Asset {
	float LoadFloat1(const json11::Json & obj);

	DirectX::XMFLOAT2 LoadFloat2(const json11::Json & obj);

	DirectX::XMFLOAT3 LoadFloat3(const json11::Json & obj);

	DirectX::XMFLOAT4 LoadFloat4(const json11::Json & obj);

	json11::Json StoreFloat2(const DirectX::XMFLOAT2 & f2);

	json11::Json StoreFloat3(const DirectX::XMFLOAT3 & f3);

	json11::Json StoreFloat4(const DirectX::XMFLOAT4 & f4);

	json11::Json StoreCollider(const Collider & collider);

	Netcode::Asset::Collider LoadCollider(const json11::Json & json);
}
