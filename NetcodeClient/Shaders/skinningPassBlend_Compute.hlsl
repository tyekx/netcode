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

groupshared float4x4 animationMatrices[MAX_BONE_COUNT];

int GetParentIndex(uint bid) {
	return parentIndices[bid / 4][bid % 4];
}

uint GetStartIndex(uint clipId) {
	return startIndices[clipId / 4][clipId % 4];
}

float GetWeight(uint instanceId, uint stateId) {
	return instances[instanceId].weights[stateId / 4][stateId % 4];
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

StructuredBuffer<BoneAnimationKey> animations : register(t0);


uint GetKeyIndex(uint numBones, uint startIndex, uint frameIndex, uint boneId) {
	return startIndex * numBones + frameIndex * numBones + boneId;
}

[RootSignature(SKINNING_PASS_ROOT_SIGNATURE)]
[numthreads(MAX_BONE_COUNT, 1, 1)]
void main(uint3 groupId : SV_GroupID, uint threadId : SV_GroupIndex) {
	uint instanceId = groupId.y;
	uint boneId = threadId;
	const uint numBones = constants.x;


	uint destIndex = GetIntermediateOffset(instanceId, 0) + boneId;
	if(boneId < numBones) {

		const uint lhsStateId = 0;
		uint numStates = instances[instanceId].numStates.x;
		float weightSum = GetWeight(instanceId, lhsStateId);
		for(uint i = 1; i < numStates; ++i) {
			float rhsW = GetWeight(instanceId, i);
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

		animationMatrices[boneId] = mul(mul(scaling, rotation), translation );
	}

	uint resultOffset = GetResultOffset(instanceId);
	uint bindTransformDestIdx = resultOffset + boneId;
	uint toRootDestIdx = resultOffset + MAX_BONE_COUNT + boneId;

	GroupMemoryBarrierWithGroupSync();

	if(boneId >= numBones) {
		return;
	}

	int parentIdx = GetParentIndex(boneId);
	float4x4 toRoot = animationMatrices[boneId];

	while(parentIdx >= 0) {
		toRoot = mul(toRoot, animationMatrices[parentIdx]);
		parentIdx = GetParentIndex(parentIdx);
	}

	float4x4 bindTransform = mul(offsetMatrices[boneId], toRoot);
	
	result[boneId] = bindTransform;
	result[toRootDestIdx] = toRoot;
}


