#pragma once

#include <NetcodeFoundation/Math.h>

namespace Netcode {

	__declspec(align(16)) struct Light {
		Float4 intensity;
		/*
		* For directional light set position.w to 0.
		*/
		Float4 position;

		float falloffStart;
		float falloffEnd;
		float spotPower;
		float __structPad0;
		Float4 direction;

	};

	struct PointLight : public Netcode::Light {
		PointLight(const Float3 & intensity, const Float4 & position, float falloffStart, float falloffEnd)
		{
			Light::intensity = Float4{ intensity.x, intensity.y, intensity.z, 0.0f };
			Light::position = position;
			Light::falloffStart = falloffStart;
			Light::falloffEnd = falloffEnd;
		}
	};

	struct DirectionalLight : public Netcode::Light {
		/*
		* For directional light set position.w to 0.
		*/
		DirectionalLight(const  Float3 & intensity, const Float4 & position)
		{
			Light::intensity = Float4{ intensity.x, intensity.y, intensity.z, 0.0f };
			Light::position = position;
		}
	};

	struct SpotLight : public Netcode::Light {
		SpotLight(const Float3 & intensity, const Float4 & position, const Float3 & direction, float falloffStart, float falloffEnd, float spotPower)
		{
			Light::intensity = Float4{ intensity.x, intensity.y, intensity.z, 0.0f };
			Light::position = position;
			Light::falloffStart = falloffStart;
			Light::falloffEnd = falloffEnd;
			Light::spotPower = spotPower;
			Light::direction = Float4{ direction.x, direction.y, direction.z, 0.0f };
		}
	};

}

struct PerFrameData {
	Netcode::Float4x4 ViewProj;
	Netcode::Float4x4 ViewProjInv;
	Netcode::Float4x4 View;
	Netcode::Float4x4 Proj;
	Netcode::Float4x4 ViewInv;
	Netcode::Float4x4 ProjInv;
	Netcode::Float4x4 ProjTex;
	Netcode::Float4x4 RayDir;
	Netcode::Float4 eyePos;
	float nearZ;
	float farZ;
	float aspectRatio;
	float fov;
};

struct BoneData {
	constexpr static int MAX_BONE_COUNT = 128;
	Netcode::Float4x4 BindTransform[MAX_BONE_COUNT];
	Netcode::Float4x4 ToRootTransform[MAX_BONE_COUNT];
};

struct BoneVisibilityData {
	uint32_t BoneVisibility[BoneData::MAX_BONE_COUNT];
};

struct PerObjectData {
	Netcode::Float4x4 Model;
	Netcode::Float4x4 InvModel;
};

struct LightData {
	Netcode::Light lights[4];
	Netcode::Float4 ambientLightIntensity;
	int numLights;
};
