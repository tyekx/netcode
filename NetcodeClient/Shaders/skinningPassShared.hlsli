static const int MAX_BONE_COUNT = 128;
static const int MAX_ACTIVE_STATE_COUNT = 8;
static const int MAX_INSTANCE_COUNT = 32;
static const int MAX_ANIMATION_COUNT = 64;

struct BoneAnimationKey {
	float4 position;
	float4 rotation;
	float4 scale;
};

struct AnimationInstance {
	uint4 numStates;
	float4 weights[MAX_ACTIVE_STATE_COUNT / 4];
	float4 normalizedTimes[MAX_ACTIVE_STATE_COUNT / 4];
	uint4 animationIndices[MAX_ACTIVE_STATE_COUNT / 4];
	uint4 frameIndices[MAX_ACTIVE_STATE_COUNT / 2];
};

uint GetIntermediateOffset(uint instanceId, uint stateId) {
	return MAX_BONE_COUNT * MAX_ACTIVE_STATE_COUNT * instanceId + MAX_BONE_COUNT * stateId;
}

uint GetResultOffset(uint instanceId) {
	return 2 * MAX_BONE_COUNT * instanceId;
}

#define SKINNING_PASS_ROOT_SIGNATURE "CBV(b0), CBV(b1), DescriptorTable(SRV(t0), UAV(u0), UAV(u1))"
