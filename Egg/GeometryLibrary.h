#pragma once

#include "Asset/Mesh.h"
#include "Geometry.h"


namespace Egg::Graphics::Internal {

	class GeometryLibrary {

		bool Exist(Asset::Mesh * mesh) {

		}

		void CreateResources(Asset::Mesh * mesh) {

		}

		Geometry * Find(Asset::Mesh * mesh) {

		}

	public:
		Geometry * GetGeometry(Asset::Mesh * mesh) {
			if(!Exist(mesh)) {
				CreateResources(mesh);
			}
			return Find(mesh);
		}
	};

}
