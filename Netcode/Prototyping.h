#pragma once

#include "Asset/Model.h"
#include "Material.h"

namespace Egg {

	class MaterialSignature {


		bool useNormals;
		bool useTexCoords;
		bool useTangentSpace;
		bool useBoneWeights;

		bool hasDiffuseTexture;
		bool hasNormalTexture;

	public:
		MaterialSignature(const Asset::Model & assetMat) noexcept {
			
		}

		bool operator==(const MaterialSignature & ms) const noexcept {
			return false;
		}

	};

	class MaterialManager {

	public:
		static Egg::Material::P CreateMaterial(const Asset::Model & model) {

		}

	};


}
