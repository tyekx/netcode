#pragma once

#include <DirectXMath.h>

namespace Netcode::Asset {

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

		Material();

		bool HasTextures() const;

		bool HasDiffuseTexture() const;

		bool HasNormalTexture() const;

		bool HasAmbientTexture() const;

		bool HasSpecularTexture() const;

		bool HasRoughnessTexture() const;

	};

}
