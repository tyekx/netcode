#pragma once

#include <DirectXMath.h>

namespace Egg {

	namespace Asset {

		struct Material {

			DirectX::XMFLOAT3 diffuseColor;
			DirectX::XMFLOAT3 ambientColor;
			DirectX::XMFLOAT3 specularColor;
			float shininess;

			char diffuseTexture[256];
			char normalTexture[256];

			Material() : diffuseColor{}, ambientColor{}, specularColor{}, shininess{} {
				diffuseTexture[0] = '\0';
				normalTexture[0] = '\0';
			}

			bool HasTextures() const {
				return HasDiffuseTexture() || HasNormalTexture();
			}

			bool HasDiffuseTexture() const {
				return diffuseTexture[0] == '\0';
			}

			bool HasNormalTexture() const {
				return normalTexture[0] == '\0';
			}

		};

	}

}
