#pragma once

#include <DirectXMath.h>
#include <json11/json11.hpp>
#include <vector>

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
	};

	struct Material {
		FbxReference source;
		json11::Json modifiedProps;

		json11::Json Store() const {
			return json11::Json::object();
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
	};

	enum class ColliderType : unsigned {
		STATIC, BONE_ATTACHED
	};

	enum class ShapeType : unsigned {
		MESH, BOX, SPHERE, CAPSULE, PLANE
	};

	struct Collider {
		ColliderType type;
		ShapeType shape;
		int boneReference;
		DirectX::XMFLOAT4 localPosition;
		DirectX::XMFLOAT4 localRotation;
		union {
			DirectX::XMFLOAT3 planeShape_Normal;
			DirectX::XMFLOAT3 boxShape_HalfExtents;
			DirectX::XMFLOAT2 capsuleShape_HalfExtentRadius;
			float sphereShape_Radius;
		};
	};

	struct Geometry {
		int materialIndex;
		std::vector<FbxReference> lods;
	};

	FbxReference base;
	std::vector<Geometry> meshes;
	std::vector<Material> materials;
	std::vector<Animation> animations;
	std::vector<Collider> colliders;

	Manifest() = default;

	void Reset() {

	}
	
	bool Load(const json11::Json & json) {

	}

	json11::Json Store() const {
		json11::Json::object obj;
		obj["name"] = "";
		obj["base"] = base.Store();

		json11::Json::array matsJson;

		std::transform(std::begin(materials), std::end(materials), std::back_inserter(matsJson), [](const Material & m) -> json11::Json {
			return m.Store();
		});

		json11::Json::array animationsJson;

		std::transform(std::begin(animations), std::end(animations), std::back_inserter(animationsJson), [](const Animation & m) ->json11::Json {
			return m.Store();
		});

		obj["materials"] = matsJson;
		obj["animations"] = animationsJson;

		return obj;
	}
};
