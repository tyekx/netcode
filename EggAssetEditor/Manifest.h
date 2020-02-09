#pragma once

#include <DirectXMath.h>
#include <json11/json11.hpp>
#include <vector>
#include "Model.h"

class Manifest {
public:
	static json11::Json::shape jsonShape_root;
	static json11::Json::shape jsonShape_Geometry;
	static json11::Json::shape jsonShape_Material;
	static json11::Json::shape jsonShape_Collider;
	static json11::Json::shape jsonShape_Animation;
	static json11::Json::shape jsonShape_FbxReference;

	struct FbxReference {
		std::string file;
		std::string reference;

		json11::Json Store() const {
			json11::Json::object obj;
			obj["file"] = file;
			obj["reference"] = reference;
			return obj;
		}

		void Load(const json11::Json & obj) {
			file = obj["file"].string_value();
			reference = obj["reference"].string_value();
		}
	};

	struct Material {
		FbxReference source;
		json11::Json modifiedProps;

		json11::Json Store() const {
			return json11::Json::object();
		}

		void Load(const json11::Json & json) {

		}
	};

	struct Animation {
		FbxReference source;
		std::string name;
		float editorPlaybackSpeed;
		bool editorPlaybackLoop;

		json11::Json Store() const {
			return json11::Json::object{
				{ "source", source.Store() },
				{ "name", name },
				{ "editor_playback_speed", editorPlaybackSpeed  },
				{ "editor_playback_loop", editorPlaybackLoop }
			};
		}

		void Load(const json11::Json & json) {
			source.Load(json["source"]);
			name = json["name"].string_value();
			editorPlaybackLoop = json["editor_playback_loop"].bool_value();
			editorPlaybackSpeed = static_cast<float>(json["editor_playback_speed"].number_value());
		}
	};

	struct Geometry {
		int materialIndex;
		std::vector<FbxReference> lods;

		json11::Json Store() const {
			json11::Json::object obj;
			json11::Json::array arr;
			obj["material_index"] = materialIndex;

			for(const FbxReference & ref : lods) {
				arr.push_back(ref.Store());
			}

			obj["lods"] = std::move(arr);

			return obj;
		}

		void Load(const json11::Json & json) {

		}
	};

	std::string name;
	FbxReference base;
	std::vector<Manifest::Geometry> meshes;
	std::vector<Manifest::Material> materials;
	std::vector<Manifest::Animation> animations;
	std::vector<Collider> colliders;

	Manifest() = default;

	void Reset() {

	}
	
	bool Load(const json11::Json & json) {
		std::string err;

		if(!json.has_shape(jsonShape_root, err)) {
			return false;
		}

		if(!json["base"].has_shape(jsonShape_FbxReference, err)) {
			return false;
		}

		for(const auto & matJson : json["materials"].array_items()) {
			if(!matJson.has_shape(jsonShape_Material, err)) {
				return false;
			}

			if(!matJson["source"].has_shape(jsonShape_FbxReference, err)) {
				return false;
			}
		}

		for(const auto & colliderJson : json["colliders"].array_items()) {
			if(!colliderJson.has_shape(jsonShape_Collider, err)) {
				return false;
			}
		}

		for(const auto & geometryJson : json["geometry"].array_items()) {
			if(!geometryJson.has_shape(jsonShape_Geometry, err)) {
				return false;
			}

			for(const auto & lodJson : geometryJson["lods"].array_items()) {
				if(!lodJson.has_shape(jsonShape_FbxReference, err)) {
					return false;
				}
			}
		}

		for(const auto & animationJson : json["animations"].array_items()) {
			if(!animationJson.has_shape(jsonShape_Animation, err)) {
				return false;
			}

			if(!animationJson["source"].has_shape(jsonShape_FbxReference, err)) {
				return false;
			}
		}

		// otherwise its a valid json and read to be committed into this object

		name = json["name"].string_value();
		base.Load(json["base"]);

		meshes.clear();
		materials.clear();
		animations.clear();
		colliders.clear();

		for(const auto & matJson : json["materials"].array_items()) {
			Manifest::Material manMat;
			manMat.Load(matJson);
			materials.push_back(manMat);
		}
		for(const auto & geomJson : json["geometry"].array_items()) {
			Manifest::Geometry manGeom;
			manGeom.Load(geomJson);
			meshes.push_back(manGeom);
		}
		for(const auto & animJson : json["animations"].array_items()) {
			Manifest::Animation manAnim;
			manAnim.Load(animJson);
			animations.push_back(manAnim);
		}
		for(const auto & collJson : json["colliders"].array_items()) {
			Collider c = LoadCollider(collJson);
			colliders.push_back(c);
		}
		return true;
	}

	static float LoadFloat1(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 1 ||
			!obj.array_items().at(0).is_number()) {
			return 0.0f;
		}

		double x = obj.array_items().at(0).number_value();

		return static_cast<float>(x);
	}

	static DirectX::XMFLOAT2 LoadFloat2(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 2 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number()) {
			return DirectX::XMFLOAT2{};
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();

		return DirectX::XMFLOAT2{ static_cast<float>(x), static_cast<float>(y) };
	}

	static DirectX::XMFLOAT3 LoadFloat3(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 3 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number() ||
			!obj.array_items().at(2).is_number()) {
			return DirectX::XMFLOAT3{};
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();
		double z = obj.array_items().at(2).number_value();

		return DirectX::XMFLOAT3{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
	}

	static DirectX::XMFLOAT4 LoadFloat4(const json11::Json & obj) {
		if(!obj.is_array() || obj.array_items().size() < 4 ||
			!obj.array_items().at(0).is_number() ||
			!obj.array_items().at(1).is_number() ||
			!obj.array_items().at(2).is_number() ||
			!obj.array_items().at(3).is_number()) {
			return DirectX::XMFLOAT4{};
		}

		double x = obj.array_items().at(0).number_value();
		double y = obj.array_items().at(1).number_value();
		double z = obj.array_items().at(2).number_value();
		double w = obj.array_items().at(3).number_value();

		return DirectX::XMFLOAT4{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w) };
	}

	static json11::Json StoreFloat2(const DirectX::XMFLOAT2 & f2) {
		json11::Json::array arr;
		arr.push_back(f2.x);
		arr.push_back(f2.y);
		return arr;
	}

	static json11::Json StoreFloat3(const DirectX::XMFLOAT3 & f3) {
		json11::Json::array arr;
		arr.push_back(f3.x);
		arr.push_back(f3.y);
		arr.push_back(f3.z);
		return arr;
	}

	static json11::Json StoreFloat4(const DirectX::XMFLOAT4 & f4) {
		json11::Json::array arr;
		arr.push_back(f4.x);
		arr.push_back(f4.y);
		arr.push_back(f4.z);
		arr.push_back(f4.w);
		return arr;
	}

	static json11::Json StoreCollider(const Collider & collider) {
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

	static Collider LoadCollider(const json11::Json & json) {
		int32_t type = json["type"].int_value();
		int32_t boneReference = json["bone_reference"].int_value();

		Collider c = {};
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

	json11::Json Store() const {
		json11::Json::object obj;
		obj["name"] = name;
		obj["base"] = base.Store();

		json11::Json::array matsJson;

		std::transform(std::begin(materials), std::end(materials), std::back_inserter(matsJson), [](const Material & m) -> json11::Json {
			return m.Store();
		});

		json11::Json::array animationsJson;

		std::transform(std::begin(animations), std::end(animations), std::back_inserter(animationsJson), [](const Animation & m) ->json11::Json {
			return m.Store();
		});

		json11::Json::array collidersJson;

		std::transform(std::begin(colliders), std::end(colliders), std::back_inserter(collidersJson), [](const Collider & m) ->json11::Json {
			return StoreCollider(m);
		});

		json11::Json::array geometryJson;

		std::transform(std::begin(meshes), std::end(meshes), std::back_inserter(geometryJson), [](const Geometry & g) -> json11::Json {
			return g.Store();
		});

		obj["materials"] = matsJson;
		obj["animations"] = animationsJson;
		obj["colliders"] = collidersJson;
		obj["geometry"] = geometryJson;

		return obj;
	}
};
