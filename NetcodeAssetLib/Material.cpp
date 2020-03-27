#include "Material.h"

namespace Netcode::Asset {

	Material::Material() : diffuseColor{}, ambientColor{}, specularColor{}, shininess{},
		diffuseTexture{}, normalTexture{}, specularTexture{}, ambientTexture{}, roughnessTexture{} {

	}

	bool Material::HasTextures() const {
		return HasDiffuseTexture() || HasNormalTexture() || HasAmbientTexture() || HasSpecularTexture() || HasRoughnessTexture();
	}

	bool Material::HasDiffuseTexture() const {
		return diffuseTexture[0] != '\0';
	}

	bool Material::HasNormalTexture() const {
		return normalTexture[0] != '\0';
	}

	bool Material::HasAmbientTexture() const {
		return ambientTexture[0] != '\0';
	}

	bool Material::HasSpecularTexture() const {
		return specularTexture[0] != '\0';
	}

	bool Material::HasRoughnessTexture() const {
		return roughnessTexture[0] != '\0';
	}

}

