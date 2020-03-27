#pragma once

#include <DirectXMath.h>

namespace Netcode {

	__declspec(align(16)) struct Light {
		DirectX::XMFLOAT3A intensity;
		/*
		* For directional light set position.w to 0.
		*/
		DirectX::XMFLOAT4A position;

		float falloffStart;
		float falloffEnd;
		float spotPower;
		float __structPad0;
		DirectX::XMFLOAT3A direction;

	};

	struct PointLight : public Netcode::Light {
		PointLight(const DirectX::XMFLOAT3A & intensity, const DirectX::XMFLOAT4A & position, float falloffStart, float falloffEnd)
		{
			Light::intensity = intensity;
			Light::position = position;
			Light::falloffStart = falloffStart;
			Light::falloffEnd = falloffEnd;
		}
	};

	struct DirectionalLight : public Netcode::Light {
		/*
		* For directional light set position.w to 0.
		*/
		DirectionalLight(const DirectX::XMFLOAT3A & intensity, const DirectX::XMFLOAT4A & position)
		{
			Light::intensity = intensity;
			Light::position = position;
		}
	};

	struct SpotLight : public Netcode::Light {
		SpotLight(const DirectX::XMFLOAT3A & intensity, const DirectX::XMFLOAT4A & position, const DirectX::XMFLOAT3A & direction,  float falloffStart, float falloffEnd, float spotPower)
		{
			Light::intensity = intensity;
			Light::position = position;
			Light::falloffStart = falloffStart;
			Light::falloffEnd = falloffEnd;
			Light::spotPower = spotPower;
			Light::direction = direction;
		}
	};

}

