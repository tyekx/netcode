#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode::Asset {

	struct Material {
		Netcode::Float4 diffuseColor;
		Netcode::Float3 ambientColor;
		Netcode::Float3 specularColor;
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
