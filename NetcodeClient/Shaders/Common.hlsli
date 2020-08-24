struct FrameData {
	float4x4 viewProj;
	float4x4 viewProjInv;
	float4x4 view;
	float4x4 proj;
	float4x4 viewInv;
	float4x4 projInv;
	float4x4 projTex;
	float4x4 rayDir;
	float4 eyePos;
	float4 ambientLightIntensity;
	float nearZ;
	float farZ;
	float aspectRatio;
	float fov;
};

// IF CHANGED: make sure everything is naturally aligned
struct Light {
	float4 intensity;
	float3 position;
	float referenceDistance;
	float3 direction;
	float minDistance;
	float maxDistance;
	float angleScale;
	float angleOffset;
	uint type;
};

struct ObjectData {
	float4x4 model;
	float4x4 invModel;
	int lightsCount;
	int lightsOffset;
};

struct BoneData {
	float4x4 bindTransforms[128];
	float4x4 toRootTransforms[128];
};
