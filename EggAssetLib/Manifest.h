#pragma once

#include <DirectXMath.h>
#include <json11/json11.hpp>
#include <vector>
#include <algorithm>
#include "JsonUtility.h"
#include "Collider.h"

namespace Egg::Asset {

	class Manifest {
	public:

		struct FbxReference {
			std::string file;
			std::string reference;

			json11::Json Store() const;

			void Load(const json11::Json & obj);
		};

		struct Material {
			FbxReference source;
			json11::Json modifiedProps;

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
