#pragma once

#include <DirectXMath.h>

namespace Egg::Asset {

	struct Material {

		DirectX::XMFLOAT4 diffuseColor;
		DirectX::XMFLOAT3 ambientColor;
		DirectX::XMFLOAT3 specularColor;
		float shininess;

		char diffuseTexture[256];
		char normalTexture[256];
		char specularTexture[256];
		char ambientTexture[256];
		char roughnessTexture[256];

		Material() : diffuseColor{}, ambientColor{}, specularColor{}, shininess{},
			diffuseTexture{}, normalTexture{}, specularTexture{}, ambientTexture{}, roughnessTexture{} {

		}

		bool HasTextures() const {
			return HasDiffuseTexture() || HasNormalTexture() || HasAmbientTexture() || HasSpecularTexture() || HasRoughnessTexture();
		}

		bool HasDiffuseTexture() const {
			return diffuseTexture[0] != '\0';
		}

		bool HasNormalTexture() const {
			return normalTexture[0] != '\0';
		}

		bool HasAmbientTexture() const {
			return ambientTexture[0] != '\0';
		}

		bool HasSpecularTexture() const {
			return specularTexture[0] != '\0';
		}

		bool HasRoughnessTexture() const {
			return roughnessTexture[0] != '\0';
		}

	};

}
