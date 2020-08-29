#pragma once

#include <DirectXMath.h>

namespace Netcode {


	enum class LightType : uint32_t {
		DIRECTIONAL, OMNI, SPOT
	};

	__declspec(align(16)) struct Light {
		Float4 intensity;
		Float3 position;
		float referenceDistance; // closer: brigher, further: dimmer
		Float3 direction;
		float minimumDistance;
		float maximumDistance;
		float angleScale;
		float angleOffset;
		LightType type;

		Light() = default;
	};

	struct OmniLight : public Netcode::Light {
		OmniLight(const Float3 & intensity, const Float3 & position, float falloffStart, float falloffEnd) : Light()
		{
			Light::intensity = Float4{ intensity.x, intensity.y, intensity.z, 0.0f };
			Light::position = position;
			Light::minimumDistance = 0.01f;
			Light::maximumDistance = falloffEnd;
			Light::type = LightType::OMNI;
			Light::referenceDistance = falloffStart;
		}
	};

	struct DirectionalLight : public Netcode::Light {
		/*
		* For directional light set position.w to 0.
		*/
		DirectionalLight(const  Float3 & intensity, const Float3 & position)
		{
			Light::intensity = Float4{ intensity.x, intensity.y, intensity.z, 0.0f };
			Light::position = position;
			Light::type = LightType::DIRECTIONAL;
			Light::referenceDistance = 0.01f;
		}
	};

	struct SpotLight : public Netcode::Light {
		SpotLight(const Float3 & intensity, const Float3 & position, const Float3 & direction, float minDist, float maxDist, float cosAngleStart, float cosAngleEnd)
		{
			Light::intensity = Float4{ intensity.x, intensity.y, intensity.z, 0.0f };
			Light::position = position;
			Light::minimumDistance = 0.01f;
			Light::maximumDistance = maxDist;
			Light::referenceDistance = minDist;
			Light::direction = direction;
			Light::type = LightType::SPOT;
			Light::angleScale = 1.0f / std::max(0.001f, (cosAngleStart - cosAngleEnd));
			Light::angleOffset = -cosAngleEnd + angleScale;
		}
	};

}

