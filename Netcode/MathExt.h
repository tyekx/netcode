#pragma once

#include <NetcodeFoundation/Math.h>
#include <physx/PxPhysicsAPI.h>
#include <rapidjson/document.h>

namespace Netcode {

	Netcode::Float4 LoadFloat4(rapidjson::Value & value);

	Netcode::Int2 LoadInt2(rapidjson::Value & value);

	Netcode::UInt2 LoadUInt2(rapidjson::Value & value);

	physx::PxVec3 ToPxVec3(const Netcode::Float3 & v);

	physx::PxVec3 ToPxVec3(const physx::PxExtendedVec3 & ev);

	physx::PxQuat ToPxQuat(const Netcode::Float4 & q);

	Netcode::Float3 ToFloat3(const physx::PxVec3 & v);

	Netcode::Float4 ToFloat4(const physx::PxVec4 & v);

	Netcode::Float4 ToFloat4(const physx::PxQuat & q);

	Netcode::Vector3 ToVec3(const physx::PxVec3 & v);

	Netcode::Vector3 ToVec3(const physx::PxExtendedVec3 & ev);

	Netcode::Vector4 ToVec4(const physx::PxVec4 & v);

	Netcode::Quaternion ToQuaternion(const physx::PxQuat & q);

	float RandomFloat();

	float RandomFloat(float a, float b);
}
