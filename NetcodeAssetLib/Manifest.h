#pragma once

#include <json11.hpp>
#include <vector>
#include "Collider.h"

namespace Netcode::Asset {

	class Manifest {
	public:

		struct FbxReference {
			std::string file;
			std::string reference;

			json11::Json Store() const;

			void Load(const json11::Json & obj);
		};

		struct Material {
			std::string name;
			int32_t type;
			json11::Json::object parameters;

			json11::Json Store() const;

			void Load(const json11::Json & json);
		};

		struct Animation {
			FbxReference source;
			std::string name;
			float editorPlaybackSpeed;
			bool editorPlaybackLoop;

			json11::Json Store() const;

			void Load(const json11::Json & json);
		};

		struct Geometry {
			int materialIndex;
			std::vector<FbxReference> lods;

			json11::Json Store() const;

			void Load(const json11::Json & json);
		};

		std::string name;
		FbxReference base;
		Netcode::Float4x4 offlineTransform;
		std::vector<Manifest::Geometry> meshes;
		std::vector<Manifest::Material> materials;
		std::vector<Manifest::Animation> animations;
		std::vector<Collider> colliders;

		Manifest() = default;

		void Reset();
	
		bool Load(const json11::Json & json);

		json11::Json Store() const;
	};

}
