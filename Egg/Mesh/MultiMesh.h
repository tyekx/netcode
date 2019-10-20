#pragma once

#include "../Common.h"
#include "Shaded.h"

namespace Egg {

	namespace Mesh {

		GG_CLASS(MultiMesh)
			std::vector<Egg::Mesh::Shaded::P> meshes;
		public:
			MultiMesh() : meshes{} { }
			~MultiMesh() = default;

			void Add(Egg::Mesh::Geometry::P geometry, Egg::Material::P material, PerMeshCb * cb) {
				meshes.push_back(Egg::Mesh::Shaded::Create(geometry, material, cb));
			}

			void Add(Egg::Mesh::Shaded::P shaded) {
				meshes.push_back(shaded);
			}

			std::vector<Egg::Mesh::Shaded::P> & GetMeshes() {
				return meshes;
			}

		GG_ENDCLASS

	}

}
