#include "quaternion.hlsli"
#include "skinningPassShared.hlsli"

RWStructuredBuffer<BoneAnimationKey> intermediate : register(u0);
RWStructuredBuffer<float4x4> result : register(u1);

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
	int parentIndices[MAX_BONE_COUNT];
	float4x4 offsetMatrices[MAX_BONE_COUNT];
}

groupshared float4x4 animationMatrices[MAX_BONE_COUNT];

[RootSignature(SKINNING_PASS_ROOT_SIGNATURE)]
[numthreads(MAX_BONE_COUNT, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadId : SV_GroupThreadID) {
	uint instanceId = groupId.y;
	uint boneId = threadId.x;

	if(boneId < numBones) {

		const uint lhsStateId = 0;
		uint numStates = instances[instanceId].numStates;
		float weightSum = instances[instanceId].weights[lhsStateId];

		uint destIndex = GetIntermediateOffset(instanceId, lhsStateId) + boneId;
		for(uint i = 1; i < numStates; ++i) {
			float rhsW = instances[instanceId].weights[i];
			uint rhsIdx = GetIntermediateOffset(instanceId, i) + boneId;

			float t = (rhsW) / (weightSum + rhsW);

			intermediate[destIndex].position = lerp(intermediate[destIndex].position, intermediate[rhsIdx].position, t);
			intermediate[destIndex].rotation = QuaternionSlerp(intermediate[destIndex].rotation, intermediate[rhsIdx].rotation, t);
			intermediate[destIndex].scale = lerp(intermediate[destIndex].scale, intermediate[rhsIdx].scale, t);
		}

		float4 p = intermediate[destIndex].position;
		float4 quat = intermediate[destIndex].rotation;
		float4 s = intermediate[destIndex].scale;

		float4x4 translation = float4x4(1.0f, 0.0f, 0.0f, 0.0f,
										0.0f, 1.0f, 0.0f, 0.0f,
										0.0f, 0.0f, 1.0f, 0.0f,
										p.x, p.y, p.z, p.w);

		float4x4 scaling = float4x4(s.x, 0.0f, 0.0f, 0.0f,
									0.0f, s.y, 0.0f, 0.0f,
									0.0f, 0.0f, s.z, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);

		float4x4 rotation = QuaternionToMatrix(quat);

		animationMatrices[boneId] = scaling * rotation * translation;
	}

	uint resultOffset = GetResultOffset(instanceId);
	uint bindTransformDestIdx = resultOffset + boneId;
	uint toRootDestIdx = resultOffset + MAX_BONE_COUNT + boneId;

	GroupMemoryBarrierWithGroupSync();

	if(boneId >= numBones) {
		return;
	}

	int parentIdx = parentIndices[boneId];
	float4x4 toRoot = animationMatrices[boneId];
	while(parentIdx > -1) {
		toRoot = mul(toRoot, animationMatrices[parentIdx]);
		parentIdx = parentIndices[parentIdx];
	}

	float4x4 bindTransform = mul(toRoot, offsetMatrices[boneId]);

	result[toRootDestIdx] = toRoot; 
	result[bindTransformDestIdx] = bindTransform;
}


