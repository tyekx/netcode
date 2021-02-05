#pragma once

#include <NetcodeFoundation/Math.h>
#include <physx/PxPhysicsAPI.h>
#include <rapidjson/document.h>

namespace Netcode {

	Float2 LoadFloat2(const rapidjson::Value & value);

	Float3 LoadFloat3(const rapidjson::Value & value);

	Float4 LoadFloat4(const rapidjson::Value & value);

	Int2 LoadInt2(const rapidjson::Value & value);

	Int3 LoadInt3(const rapidjson::Value & value);

	Int4 LoadInt4(const rapidjson::Value & value);

	UInt2 LoadUInt2(const rapidjson::Value & value);

	UInt3 LoadUInt3(const rapidjson::Value & value);

	UInt4 LoadUInt4(const rapidjson::Value & value);
	
	physx::PxVec3 ToPxVec3(const Netcode::Float3 & v);

	physx::PxVec3 ToPxVec3(const physx::PxExtendedVec3 & ev);

	physx::PxExtendedVec3 ToPxExtVec3(const Float3 & v);

	physx::PxQuat ToPxQuat(const Netcode::Float4 & q);

	Float3 ToFloat3(const physx::PxVec3 & v);

	Float4 ToFloat4(const physx::PxVec4 & v);

	Float4 ToFloat4(const physx::PxQuat & q);

	Vector3 ToVec3(const physx::PxVec3 & v);

	Vector3 ToVec3(const physx::PxExtendedVec3 & ev);

	Vector4 ToVec4(const physx::PxVec4 & v);

	Quaternion ToQuaternion(const physx::PxQuat & q);

	float RandomFloat();

	float RandomFloat(float a, float b);
}
