#pragma once

#include <DirectXMath.h>

namespace Egg {

	namespace Asset {

		struct Material {

			DirectX::XMFLOAT3 diffuseColor;
			DirectX::XMFLOAT3 ambientColor;
			DirectX::XMFLOAT3 specularColor;
			float shininess;

			char diffuseTexture[64];
			char normalTexture[64];

			Material() : diffuseColor{}, ambientColor{}, specularColor{}, shininess{} {
				diffuseTexture[0] = '\0';
				normalTexture[0] = '\0';
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
