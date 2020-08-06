#include "Manifest.h"
#include <algorithm>
#include <iterator>
#include "JsonUtility.h"


namespace Netcode::Asset {

	static json11::Json::shape jsonShape_root = {
		{ "name", json11::Json::Type::STRING },
		{ "base",  json11::Json::Type::OBJECT },
		{ "materials", json11::Json::Type::ARRAY },
		{ "colliders", json11::Json::Type::ARRAY },
		{ "animations", json11::Json::Type::ARRAY },
		{ "geometry", json11::Json::Type::ARRAY },
	};

	static json11::Json::shape jsonShape_Geometry = {
		{ "materialIndex", json11::Json::Type::NUMBER },
		{ "lods", json11::Json::Type::ARRAY }
	};

	static json11::Json::shape jsonShape_Material = {
		{ "source", json11::Json::Type::OBJECT },
		{ "modified_props", json11::Json::Type::OBJECT }
	};

	static json11::Json::shape jsonShape_Collider = {
		{ "type", json11::Json::Type::NUMBER },
		{ "bone_reference", json11::Json::Type::NUMBER },
		{ "local_position", json11::Json::Type::ARRAY },
		{ "local_rotation", json11::Json::Type::ARRAY },
		{ "shape_data", json11::Json::Type::ARRAY }
	};

	static json11::Json::shape jsonShape_Animation = {
		{ "source", json11::Json::Type::OBJECT },
		{ "editor_playback_speed", json11::Json::Type::NUMBER },
		{ "editor_playback_loop",json11::Json::Type::BOOL }
	};

	static json11::Json::shape jsonShape_FbxReference = {
		{ "file", json11::Json::Type::STRING },
		{ "reference", json11::Json::Type::STRING }
	};
	void Manifest::Reset() {

	}

	bool Manifest::Load(const json11::Json & json) {
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


	json11::Json Manifest::Store() const {
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

	json11::Json Manifest::FbxReference::Store() const {
		json11::Json::object obj;
		obj["file"] = file;
		obj["reference"] = reference;
		return obj;
	}

	void Manifest::FbxReference::Load(const json11::Json & obj) {
		file = obj["file"].string_value();
		reference = obj["reference"].string_value();
	}

	json11::Json Manifest::Material::Store() const {
		return json11::Json::object();
	}

	void Manifest::Material::Load(const json11::Json & json) {

	}

	json11::Json Manifest::Animation::Store() const {
		return json11::Json::object{
			{ "source", source.Store() },
			{ "name", name },
			{ "editor_playback_speed", editorPlaybackSpeed },
			{ "editor_playback_loop", editorPlaybackLoop }
		};
	}

	void Manifest::Animation::Load(const json11::Json & json) {
		source.Load(json["source"]);
		name = json["name"].string_value();
		editorPlaybackLoop = json["editor_playback_loop"].bool_value();
		editorPlaybackSpeed = static_cast<float>(json["editor_playback_speed"].number_value());
	}

	json11::Json Manifest::Geometry::Store() const {
		json11::Json::object obj;
		json11::Json::array arr;
		obj["material_index"] = materialIndex;

		for(const FbxReference & ref : lods) {
			arr.push_back(ref.Store());
		}

		obj["lods"] = std::move(arr);

		return obj;
	}

	void Manifest::Geometry::Load(const json11::Json & json) {

	}

}