#pragma once

#include <NetcodeFoundation/Math.h>
#include <NetcodeFoundation/Json.h>

#include <json11.hpp>
#include "Collider.h"

/*
Helper functions for primitive type serialization
*/

namespace Netcode::Asset {

	Netcode::Json::Value Store(const Netcode::Float4& value, Netcode::Json::Document::AllocatorType & allocator);
	Netcode::Json::Value Store(const Netcode::Float3& value, Netcode::Json::Document::AllocatorType & allocator);
	Netcode::Json::Value Store(const Netcode::Float2& value, Netcode::Json::Document::AllocatorType & allocator);
	Netcode::Json::Value Store(const Collider & collider, Netcode::Json::Document::AllocatorType & allocator);

	float LoadFloat1(const json11::Json & obj);

	Netcode::Float2 LoadFloat2(const json11::Json & obj);

	Netcode::Float3 LoadFloat3(const json11::Json & obj);

	Netcode::Float4 LoadFloat4(const json11::Json & obj);

	Netcode::Float4x4 LoadFloat4x4(const json11::Json & obj);

	json11::Json StoreFloat2(const Netcode::Float2 & f2);

	json11::Json StoreFloat3(const Netcode::Float3 & f3);

	json11::Json StoreFloat4(const Netcode::Float4 & f4);

	json11::Json StoreCollider(const Collider & collider);

	json11::Json StoreFloat4x4(const Netcode::Float4x4 & f44);

	Netcode::Asset::Collider LoadCollider(const json11::Json & json);
}
