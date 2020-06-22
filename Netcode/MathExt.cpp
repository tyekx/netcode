#include "MathExt.h"

namespace Netcode {

	Netcode::Float4 LoadFloat4(const rapidjson::Value & value) {
		return Netcode::Float4{
			value[0].GetFloat(),
			value[1].GetFloat(),
			value[2].GetFloat(),
			value[3].GetFloat()
		};
	}

	Netcode::Int2 LoadInt2(const rapidjson::Value & value) {
		return Netcode::Int2{
			value[0].GetInt(),
			value[1].GetInt()
		};
	}

	Netcode::UInt2 LoadUInt2(const rapidjson::Value & value) {
		return Netcode::UInt2{
			value[0].GetUint(),
			value[1].GetUint()
		};
	}

	physx::PxQuat ToPxQuat(const Netcode::Float4 & q) {
		return physx::PxQuat{ q.x, q.y, q.z, q.w };
	}

	physx::PxVec3 ToPxVec3(const Netcode::Float3 & v) {
		return physx::PxVec3{ v.x, v.y, v.z };
	}

	physx::PxVec3 ToPxVec3(const physx::PxExtendedVec3 & ev)
	{
		return physx::PxVec3(static_cast<float>(ev.x), static_cast<float>(ev.y), static_cast<float>(ev.z));
	}

	Netcode::Float3 ToFloat3(const physx::PxVec3 & v) {
		return Netcode::Float3{ v.x, v.y, v.z };
	}

	Netcode::Float4 ToFloat4(const physx::PxVec4 & v) {
		return Netcode::Float4{ v.x, v.y, v.z, v.w };
	}

	Netcode::Float4 ToFloat4(const physx::PxQuat & q) {
		return Netcode::Float4{ q.x, q.y, q.z, q.w };
	}

	Netcode::Vector3 ToVec3(const physx::PxVec3 & v) {
		return ToFloat3(v);
	}

	Netcode::Vector3 ToVec3(const physx::PxExtendedVec3 & ev) {
		return Netcode::Float3{ static_cast<float>(ev.x), static_cast<float>(ev.y), static_cast<float>(ev.z) };
	}

	Netcode::Vector4 ToVec4(const physx::PxVec4 & v) {
		return ToFloat4(v);
	}

	Netcode::Quaternion ToQuaternion(const physx::PxQuat & q) {
		return ToFloat4(q);
	}

	float RandomFloat()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	float RandomFloat(float a, float b)
	{
		return a + RandomFloat() * (b - a);
	}

}
