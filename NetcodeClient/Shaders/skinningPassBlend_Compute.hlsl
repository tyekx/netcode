#include "quaternion.hlsli"
#include "skinningPassShared.hlsli"

RWStructuredBuffer<BoneAnimationKey> intermediate : register(u0);
RWStructuredBuffer<float4x4> result : register(u1);
 
cbuffer AnimationInstanceData : register(b0) {
	uint4 numInstances;
	AnimationInstance instances[MAX_INSTANCE_COUNT];
}

cbuffer StaticConstants : register(b1) {
	uint4 constants;
	uint4 startIndices[MAX_ANIMATION_COUNT / 4];
	int4 parentIndices[MAX_BONE_COUNT / 4];
	float4x4 offsetMatrices[MAX_BONE_COUNT];
}

int GetParentIndex(uint bid) {
	return parentIndices[bid / 4][bid % 4];
}

uint GetStartIndex(uint clipId) {
	return startIndices[clipId / 4][clipId % 4];
}

float GetWeight(uint instanceId, uint sid) {
	return instances[instanceId].weights[sid / 4][sid % 4];
}

uint GetFrameIndex(uint iid, uint idx) {
	return instances[iid].frameIndices[idx / 4][idx % 4];
}

uint GetAnimationIndex(uint iid, uint idx) {
	return instances[iid].animationIndices[idx / 4][idx % 4];
}

float GetNormalizedTime(uint iid, uint idx) {
	return instances[iid].normalizedTimes[idx / 4][idx % 4];
}

uint GetKeyIndex(uint numBones, uint startIndex, uint frameIndex, uint boneId) {
	return startIndex * numBones + frameIndex * numBones + boneId;
}

StructuredBuffer<BoneAnimationKey> animations : register(t0);
groupshared float4x4 animationMatrices[MAX_BONE_COUNT];

float4x4 AffineTransformation(float4 scale, float4 quat, float4 hPos) {
	float4x4 translation = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		hPos.x, hPos.y, hPos.z, hPos.w);

	float4x4 scaling = float4x4(scale.x, 0.0f, 0.0f, 0.0f,
		0.0f, scale.y, 0.0f, 0.0f,
		0.0f, 0.0f, scale.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	float4x4 rotation = QuaternionToMatrix(quat);

	return mul(mul(scaling, rotation), translation);
}

BoneAnimationKey Interpolate(BoneAnimationKey k0, BoneAnimationKey k1, float t) {
	float4 pos = lerp(k0.position, k1.position, t);
	float4 quat = QuaternionSlerp(k0.rotation, k1.rotation, t);
	float4 scale = lerp(k0.scale, k1.scale, t);
	BoneAnimationKey key;
	key.position = pos;
	key.rotation = quat;
	key.scale = scale;
	return key;
}

void ReduceState(uint idx0, uint idx1, float t) {
	BoneAnimationKey k0 = intermediate[idx0];
	BoneAnimationKey k1 = intermediate[idx1];

	intermediate[idx0] = Interpolate(k0, k1, t);
}

[RootSignature(SKINNING_PASS_ROOT_SIGNATURE)]
[numthreads(MAX_BONE_COUNT, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint threadId : SV_GroupIndex) {
	const uint instanceId = groupId.y;
	const uint boneId = threadId;
	const uint numBones = constants.x;

	if(boneId < numBones) {
		const uint destIndex = GetIntermediateOffset(instanceId, 0) + boneId;

		uint numStates = instances[instanceId].numStates.x;
		float weightSum = GetWeight(instanceId, 0);

		for(uint i = 1; i < numStates; ++i) {
			float rhsW = GetWeight(instanceId, i);
			uint rhsIdx = GetIntermediateOffset(instanceId, i) + boneId;

			float t = (rhsW) / (weightSum + rhsW);

			ReduceState(destIndex, rhsIdx, t);

			weightSum += rhsW;
		}

		BoneAnimationKey bk = intermediate[destIndex];

		animationMatrices[boneId] = AffineTransformation(bk.scale, bk.rotation, bk.position);
	}

	GroupMemoryBarrierWithGroupSync();

	if(boneId >= numBones) {
		return;
	}

	uint resultOffset = GetResultOffset(instanceId);
	uint bindTransformDestIdx = resultOffset + boneId;
	uint toRootDestIdx = resultOffset + MAX_BONE_COUNT + boneId;

	int parentIdx = GetParentIndex(boneId);
	float4x4 toRoot = animationMatrices[boneId];

	while(parentIdx >= 0) {
		toRoot = mul(toRoot, animationMatrices[parentIdx]);
		parentIdx = GetParentIndex(parentIdx);
	}

	result[bindTransformDestIdx] = mul(offsetMatrices[boneId], toRoot);
	result[toRootDestIdx] = toRoot;
}


