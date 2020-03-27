#include "JsonUtility.h"

namespace Netcode::Asset {

	float LoadFloat1(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 1 ||
			!obj.array_items().at(0).is_number()) {
			return 0.0f;
		}

		double x = obj.array_items().at(0).number_value();

		return static_cast<float>(x);
	}

	DirectX::XMFLOAT2 LoadFloat2(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 2 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number()) {
			return DirectX::XMFLOAT2{ };
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();

		return DirectX::XMFLOAT2{ static_cast<float>(x), static_cast<float>(y) };
	}

	DirectX::XMFLOAT3 LoadFloat3(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 3 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number() ||
			!obj.array_items().at(2).is_number()) {
			return DirectX::XMFLOAT3{ };
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();
		double z = obj.array_items().at(2).number_value();

		return DirectX::XMFLOAT3{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
	}

	DirectX::XMFLOAT4 LoadFloat4(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 4 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number() ||
			!obj.array_items().at(2).is_number() ||
			!obj.array_items().at(3).is_number()) {
			return DirectX::XMFLOAT4{ };
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();
		double z = obj.array_items().at(2).number_value();
		double w = obj.array_items().at(3).number_value();

		return DirectX::XMFLOAT4{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) };
	}

	json11::Json StoreFloat2(const DirectX::XMFLOAT2 & f2) {
		json11::Json::array arr;
		arr.push_back(f2.x);
		arr.push_back(f2.y);
		return arr;
	}

	json11::Json StoreFloat3(const DirectX::XMFLOAT3 & f3) {
		json11::Json::array arr;
		arr.push_back(f3.x);
		arr.push_back(f3.y);
		arr.push_back(f3.z);
		return arr;
	}

	json11::Json StoreFloat4(const DirectX::XMFLOAT4 & f4) {
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
		}

		return obj;
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
		}

		return c;
	}

}
