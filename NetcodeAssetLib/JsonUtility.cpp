#include "JsonUtility.h"

namespace Netcode::Asset {
	/*
	Netcode::Json::Value Store(const Netcode::Float4 & value, Netcode::Json::Document::AllocatorType & allocator)
	{
		Netcode::Json::Value v{ rapidjson::Type::kArrayType };
		v.PushBack(Netcode::Json::Value{ value.x }.Move(), allocator);
		v.PushBack(Netcode::Json::Value{ value.y }.Move(), allocator);
		v.PushBack(Netcode::Json::Value{ value.z }.Move(), allocator);
		v.PushBack(Netcode::Json::Value{ value.w }.Move(), allocator);
		return v;
	}

	Netcode::Json::Value Store(const Netcode::Float3 & value, Netcode::Json::Document::AllocatorType & allocator)
	{
		Netcode::Json::Value v{ rapidjson::Type::kArrayType };
		v.PushBack(Netcode::Json::Value{ value.x }.Move(), allocator);
		v.PushBack(Netcode::Json::Value{ value.y }.Move(), allocator);
		v.PushBack(Netcode::Json::Value{ value.z }.Move(), allocator);
		return v;
	}

	Netcode::Json::Value Store(const Netcode::Float2 & value, Netcode::Json::Document::AllocatorType & allocator)
	{
		Netcode::Json::Value v{ rapidjson::Type::kArrayType };
		v.PushBack(Netcode::Json::Value{ value.x }.Move(), allocator);
		v.PushBack(Netcode::Json::Value{ value.y }.Move(), allocator);
		return v;
	}

	Netcode::Json::Value Store(const Collider & collider, Netcode::Json::Document::AllocatorType & allocator)
	{
		Netcode::Json::Value obj{ rapidjson::Type::kObjectType };
		obj.AddMember(L"type", static_cast<int32_t>(collider.type), allocator);
		obj.AddMember(L"bone_reference", static_cast<int32_t>(collider.boneReference), allocator);
		obj.AddMember(L"local_position", Store(collider.localPosition, allocator).Move(), allocator);
		obj.AddMember(L"local_rotation", Store(collider.localRotation, allocator).Move(), allocator);

		switch(collider.type) {
			case ColliderType::BOX: {
				obj.AddMember(L"shape_data", Store(collider.boxArgs, allocator).Move(), allocator);
			} break;
			case ColliderType::CAPSULE: {
				obj.AddMember(L"shape_data", Store(collider.capsuleArgs, allocator).Move(), allocator);
			} break;
			case ColliderType::SPHERE: {
				obj.AddMember(L"shape_data", Netcode::Json::Value{ collider.sphereArgs }.Move(), allocator);
			} break;
			default: break;
		}

		return obj;
	}*/

	float LoadFloat1(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 1 ||
			!obj.array_items().at(0).is_number()) {
			return 0.0f;
		}

		double x = obj.array_items().at(0).number_value();

		return static_cast<float>(x);
	}

	Netcode::Float2 LoadFloat2(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 2 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number()) {
			return Netcode::Float2{ };
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();

		return Netcode::Float2{ static_cast<float>(x), static_cast<float>(y) };
	}

	Netcode::Float3 LoadFloat3(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 3 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number() ||
			!obj.array_items().at(2).is_number()) {
			return Netcode::Float3{ };
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();
		double z = obj.array_items().at(2).number_value();

		return Netcode::Float3{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
	}

	Netcode::Float4 LoadFloat4(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 4 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number() ||
			!obj.array_items().at(2).is_number() ||
			!obj.array_items().at(3).is_number()) {
			return Netcode::Float4{ };
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();
		double z = obj.array_items().at(2).number_value();
		double w = obj.array_items().at(3).number_value();

		return Netcode::Float4{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) };
	}

	Netcode::Float4x4 LoadFloat4x4(const json11::Json & obj)
	{
		const auto & arr = obj.array_items();
		Netcode::Float4x4 m;
		for(uint32_t i = 0; i < 16; ++i) {
			m.m[i / 4][i % 4] = static_cast<float>(arr.at(i).number_value());
		}
		return m;
	}

	json11::Json StoreFloat2(const Netcode::Float2 & f2) {
		json11::Json::array arr;
		arr.push_back(f2.x);
		arr.push_back(f2.y);
		return arr;
	}

	json11::Json StoreFloat3(const Netcode::Float3 & f3) {
		json11::Json::array arr;
		arr.push_back(f3.x);
		arr.push_back(f3.y);
		arr.push_back(f3.z);
		return arr;
	}

	json11::Json StoreFloat4(const Netcode::Float4 & f4) {
		json11::Json::array arr;
		arr.push_back(f4.x);
		arr.push_back(f4.y);
		arr.push_back(f4.z);
		arr.push_back(f4.w);
		return arr;
	}

	json11::Json StoreCollider(const Collider & collider) {
		json11::Json::object obj;
		obj["type"] = static_cast<int32_t>(collider.type);
		obj["bone_reference"] = static_cast<int32_t>(collider.boneReference);
		obj["local_position"] = StoreFloat3(collider.localPosition);
		obj["local_rotation"] = StoreFloat4(collider.localRotation);

		switch(collider.type) {
			case ColliderType::BOX:
			{
				obj["shape_data"] = StoreFloat3(collider.boxArgs);
			}
			break;
			case ColliderType::CAPSULE:
			{
				obj["shape_data"] = StoreFloat2(collider.capsuleArgs);
			}
			break;
			case ColliderType::SPHERE:
			{
				json11::Json::array arr;
				arr.push_back(collider.sphereArgs);
				obj["shape_data"] = std::move(arr);
			}
			break;
			default: break;
		}

		return obj;
	}

	json11::Json StoreFloat4x4(const Netcode::Float4x4 & f44)
	{
		json11::Json::array arr;
		arr.reserve(16);
		for(uint32_t i = 0; i < 16; ++i) {
			arr.push_back(f44.m[i / 4][i % 4]);
		}
		return arr;
	}

	Netcode::Asset::Collider LoadCollider(const json11::Json & json) {
		int32_t type = json["type"].int_value();
		int32_t boneReference = json["bone_reference"].int_value();

		Collider c = { };
		c.type = static_cast<ColliderType>(type);
		c.boneReference = boneReference;
		c.localPosition = LoadFloat3(json["local_position"]);
		c.localRotation = LoadFloat4(json["local_rotation"]);

		switch(c.type) {
			case ColliderType::BOX:
			{
				c.boxArgs = LoadFloat3(json["shape_data"]);
			}
			break;
			case ColliderType::CAPSULE:
			{
				c.capsuleArgs = LoadFloat2(json["shape_data"]);
			}
			break;
			case ColliderType::SPHERE:
			{
				c.sphereArgs = LoadFloat1(json["shape_data"]);
			}
			break;
			default: break;
		}

		return c;
	}

}
