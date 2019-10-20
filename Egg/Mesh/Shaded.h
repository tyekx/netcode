#pragma once

#include "../Common.h"
#include "Geometry.h"
#include "../Material.h"
#include "../ConstantBufferTypes.h"


namespace Egg {
	namespace Mesh {

		GG_CLASS(Shaded)

			Geometry::P geometry;
			Material::P material;
			PerMeshCb * perMeshData;
		public:
			Shaded(Geometry::P geom, Material::P mat, PerMeshCb * cb) :
				geometry{ geom }, material{ mat }, perMeshData{ cb } { }

			~Shaded() = default;

			Geometry::P GetGeometry() {
				return geometry;
			}

			Material::P GetMaterial() {
				return material;
			}

			PerMeshCb * GetMeshData() {
				return perMeshData;
			}

		GG_ENDCLASS


	}
}
