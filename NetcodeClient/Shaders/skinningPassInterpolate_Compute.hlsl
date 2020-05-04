#include "quaternion.hlsli"
#include "skinningPassShared.hlsli"

StructuredBuffer<BoneAnimationKey> animations : register(t0);
RWStructuredBuffer<BoneAnimationKey> intermediate : register(u0);
 
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

uint GetKeyIndex(uint numBones, uint startIndex, uint frameIndex, uint boneId) {
	return startIndex * numBones + frameIndex * numBones + boneId;
}

uint GetStartIndex(uint clipId) {
	return startIndices[clipId / 4][clipId % 4];
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

[RootSignature(SKINNING_PASS_ROOT_SIGNATURE)]
[numthreads(MAX_BONE_COUNT, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint threadId : SV_GroupIndex) {
	uint stateId = groupId.x;
	uint instanceId = groupId.y;
	uint boneId = threadId;
	const uint numBones = constants.x;

	if(instances[instanceId].numStates.x <= stateId) {
		return;
	}

	if(boneId >= numBones) {
		return;
	}

	uint animationIndex = GetAnimationIndex(instanceId, stateId);
	uint frameIdxA = GetFrameIndex(instanceId, 2 * stateId);
	uint frameIdxB = GetFrameIndex(instanceId, 2 * stateId + 1);

	uint startIndex = GetStartIndex(animationIndex);

	uint keyIdxA = GetKeyIndex(numBones, startIndex, frameIdxA, boneId);
	uint keyIdxB = GetKeyIndex(numBones, startIndex, frameIdxB, boneId);

	BoneAnimationKey boneFrameA = animations[keyIdxA];
	BoneAnimationKey boneFrameB = animations[keyIdxB];

	float alpha = GetNormalizedTime(instanceId, stateId);

	BoneAnimationKey interpolated = Interpolate(boneFrameA, boneFrameB, alpha);

	uint intermediateOffset = GetIntermediateOffset(instanceId, stateId);

	intermediate[intermediateOffset + boneId] = interpolated;
}
