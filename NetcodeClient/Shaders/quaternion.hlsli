// [ (Q2.w * Q1.x) + (Q2.x * Q1.w) + (Q2.y * Q1.z) - (Q2.z * Q1.y),
//   (Q2.w * Q1.y) - (Q2.x * Q1.z) + (Q2.y * Q1.w) + (Q2.z * Q1.x),
//   (Q2.w * Q1.z) + (Q2.x * Q1.y) - (Q2.y * Q1.x) + (Q2.z * Q1.w),
//   (Q2.w * Q1.w) - (Q2.x * Q1.x) - (Q2.y * Q1.y) - (Q2.z * Q1.z) ]
float4 QuaternionMultiply(float4 q0, float4 q1) {
	return float4(
			dot(q1.wxy, q0.xwz) - q1.z * q0.y,
			dot(q1.wyz, q0.ywx) - q1.x * q0.z,
			dot(q1.wxz, q0.zyw) - q1.x * q0.y,
			dot(q1.wxyz, -q0.wxyz)
		);
}

static const uint SIGN_MASK = 0x80000000;

// Result = q0 * sin((1.0 - t) * Omega) / sin(Omega) + q1 * sin(t * Omega) / sin(Omega)
// where omega is angle between the vectors acquired from cos(Omega) = dot(q0, q1)
// other notes:
// - when q0, q1 is too close, lerp will be the approximation
// - always the shorter path is chosen on the 4D unit sphere
// - assumes that both quaternions are unit length
float4 QuaternionSlerp(float4 q0, float4 q1, float t) {
	float directWayLength = length(q0 - q1);
	float oppositeWayLength = length(q0 + q1);

	if(directWayLength > oppositeWayLength) {
		q1 = -q1;
	}

	float cosOmega = dot(q0, q1);

	if(cosOmega > (0.999f)) {
		return normalize(lerp(q0, q1, t));
	}

	float omega = acos(cosOmega);
	float sinOmega = sin(omega);
	float sinTOmega = sin(t * omega);
	float sinOneMinusTOmega = sin((1.0f - t) * omega);

	return normalize((sinOneMinusTOmega * q0 + sinTOmega * q1) / sinOmega);
}

float4x4 QuaternionToMatrix(float4 q) {
	float4x4 v = float4x4(
		1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z, 2.0f * q.x * q.y + 2.0f * q.z * q.w, 2.0f * q.x * q.z - 2.0f * q.y * q.w, 0.0f,
		2.0f * q.x * q.y - 2.0f * q.z * q.w, 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z, 2.0f * q.y * q.z + 2.0f * q.x * q.w, 0.0f,
		2.0f * q.x * q.z + 2.0f * q.y * q.w, 2.0f * q.y * q.z - 2.0f * q.x * q.w, 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y, 0.0f,
		0.0f,								 0.0f,								  0.0f,										  1.0f
		);

	return v;
}
