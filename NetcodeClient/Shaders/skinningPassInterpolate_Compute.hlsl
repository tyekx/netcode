#include "quaternion.hlsli"
#include "skinningPassShared.hlsli"

StructuredBuffer<BoneAnimationKey> animations : register(t0);
RWStructuredBuffer<BoneAnimationKey> intermediate : register(u0);

cbuffer AnimationInstanceData : register(b0) {
	uint numInstances;
	uint3 __padding_AnimationInstanceData;
	AnimationInstance instances[MAX_INSTANCE_COUNT];
}

cbuffer StaticConstants : register(b1) {
	uint numBones;
	uint numAnimations;
	uint2 __padding_StaticConstants;
	uint startIndices[MAX_ANIMATION_COUNT];
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

uint GetKeyIndex(uint startIndex, uint frameIndex, uint boneId) {
	return startIndex + numBones * frameIndex + boneId;
}

[RootSignature(SKINNING_PASS_ROOT_SIGNATURE)]
[numthreads(MAX_BONE_COUNT, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadId : SV_GroupThreadID) {
	uint stateId = groupId.x;
	uint instanceId = groupId.y;
	uint boneId = threadId.x;

	if(instances[instanceId].numStates <= stateId) {
		return;
	}

	if(boneId >= numBones) {
		return;
	}

	uint animationIndex = instances[instanceId].animationIndices[stateId];
	uint frameIdxA = instances[instanceId].frameIndices[2 * stateId];
	uint frameIdxB = instances[instanceId].frameIndices[2 * stateId + 1];

	uint startIndex = startIndices[animationIndex];

	uint keyIdxA = GetKeyIndex(startIndex, frameIdxA, boneId);
	uint keyIdxB = GetKeyIndex(startIndex, frameIdxB, boneId);

	BoneAnimationKey boneFrameA = animations[keyIdxA];
	BoneAnimationKey boneFrameB = animations[keyIdxB];

	float alpha = instances[instanceId].normalizedTimes[stateId];

	BoneAnimationKey interpolated = Interpolate(boneFrameA, boneFrameB, alpha);

	uint intermediateOffset = GetIntermediateOffset(instanceId, stateId);

	intermediate[intermediateOffset + boneId] = interpolated;
}
