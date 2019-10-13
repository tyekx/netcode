#pragma once

#include "../Common.h"
#include "Geometry.h"
#include "../Material.h"


namespace Egg {
	namespace Mesh {

		GG_CLASS(Shaded)

			Geometry::P geometry;
			Material::P material;
		public:
			Shaded(Geometry::P geom, Material::P mat) :
				geometry{ geom }, material{ mat } { }

			~Shaded() = default;

			Geometry::P GetGeometry() {
				return geometry;
			}

			Material::P GetMaterial() {
				return material;
			}

		GG_ENDCLASS


	}
}
