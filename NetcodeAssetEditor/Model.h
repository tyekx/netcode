#pragma once

#include <string>
#include <memory>
#include <vector>
#include <DirectXCollision.h>
#include <dxgiformat.h>
#include <NetcodeAssetLib/Collider.h>
#include <NetcodeAssetLib/Animation.h>

struct LOD {
	size_t vertexDataSizeInBytes;
	size_t indexDataSizeInBytes;
	uint32_t vertexCount;
	uint32_t indexCount;
	std::unique_ptr<uint8_t[]> vertexData;
	std::unique_ptr<uint8_t[]> indexData;
};

struct InputElement {
	std::string semanticName;
	int semanticIndex;
	DXGI_FORMAT format;
	uint32_t byteOffset;
};

struct VertexWeight {
	uint32_t vertexIndex;
	float weight;
};

struct Bone {
	DirectX::XMFLOAT4X4 transform;
	std::vector<VertexWeight> envelope;
	std::string boneName;

	bool operator!=(const Bone & rhs) const {
		return !operator==(rhs);
	}

	bool operator==(const Bone & rhs) const {
		return boneName == rhs.boneName &&
			memcmp(transform.m, rhs.transform.m, sizeof(DirectX::XMFLOAT4X4)) == 0;
	}
};

struct Mesh {
	std::string name;
	std::vector<LOD> lods;
	std::vector<InputElement> inputLayout;
	std::vector<Bone> bones;
	uint32_t vertexStride;
	uint32_t materialIdx;
	DirectX::BoundingBox boundingBox;

	static DirectX::BoundingBox CalculateBoundingBox(const Mesh & mesh);

	Mesh() = default;
	Mesh(Mesh &&) noexcept = default;
	Mesh & operator=(Mesh &&) noexcept = default;
	Mesh & operator=(const Mesh &) noexcept = delete;
	Mesh(const Mesh & rhs) = delete;
	~Mesh() noexcept = default;

	Mesh Clone() const;

	void ApplyTransformation(const Netcode::Float4x4 & mat);
};

struct Material {
	std::string name;
	Netcode::Float4 diffuseColor;
	Netcode::Float3 fresnelR0;
	float shininess;
	std::wstring diffuseMapReference;
	std::wstring normalMapReference;
	std::wstring specularMapReference;
	std::wstring ambientMapReference;
	std::wstring roughnessMapReference;
	std::wstring heightMapReference;
};

enum class AnimationEdge {
	DEFAULT = 0,
	CONSTANT = 1,
	LINEAR = 2,
	REPEAT = 3
};


struct PositionKey {
	Netcode::Float3 position;
	double time;
};

struct RotationKey {
	Netcode::Float4 rotation;
	double time;
};

struct ScaleKey {
	Netcode::Float3 scale;
	double time;
};

struct BoneAnimation {
	int BoneId;

	AnimationEdge PreState;
	AnimationEdge PostState;

	std::vector<PositionKey> positionKeys;
	std::vector<RotationKey> rotationKeys;
	std::vector<ScaleKey> scaleKeys;
};

using OptimizedAnimationKey = Netcode::Asset::AnimationKey;

struct Animation {
	double duration;
	double framesPerSecond;
	std::string name;
	std::vector<BoneAnimation> boneAnimations;
};

struct OptimizedBoneAnimation {
	std::vector<OptimizedAnimationKey> boneData;
};

struct OptimizedAnimation {
	std::string name;

	float duration;
	float framesPerSecond;
	std::vector<float> keyTimes;
	std::vector<OptimizedBoneAnimation> keys;
};

struct SkeletonBone {
	Netcode::Float4x4 transform;
	int parentIndex;
	std::string boneName;
};

struct Skeleton {
	std::vector<SkeletonBone> bones;

	int GetBoneIndex(const char * name) const {
		int idx = 0;
		for(const auto & i : bones) {
			if(i.boneName == name) {
				return idx;
			}
			++idx;
		}
		return -1;
	}
};

using Netcode::Asset::ColliderType;
using Netcode::Asset::Collider;

class Model {
public:
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::vector<OptimizedAnimation> animations;
	std::vector<Collider> colliders;
	Skeleton skeleton;
};
