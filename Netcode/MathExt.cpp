#include "MathExt.h"

namespace Netcode {

	Float2 LoadFloat2(const rapidjson::Value & value)
	{
		return Float2{
			value[0].GetFloat(),
			value[1].GetFloat()
		};
	}

	Float3 LoadFloat3(const rapidjson::Value & value)
	{
		return Float3{
			value[0].GetFloat(),
			value[1].GetFloat(),
			value[2].GetFloat()
		};
	}

	Float4 LoadFloat4(const rapidjson::Value & value) {
		return Float4{
			value[0].GetFloat(),
			value[1].GetFloat(),
			value[2].GetFloat(),
			value[3].GetFloat()
		};
	}

	Int2 LoadInt2(const rapidjson::Value & value) {
		return Int2{
			value[0].GetInt(),
			value[1].GetInt()
		};
	}

	Int3 LoadInt3(const rapidjson::Value & value)
	{
		return Int3{
			value[0].GetInt(),
			value[1].GetInt(),
			value[2].GetInt()
		};
	}

	Int4 LoadInt4(const rapidjson::Value & value)
	{
		return Int4{
			value[0].GetInt(),
			value[1].GetInt(),
			value[2].GetInt(),
			value[3].GetInt()
		};
	}

	UInt2 LoadUInt2(const rapidjson::Value & value) {
		return UInt2{
			value[0].GetUint(),
			value[1].GetUint()
		};
	}

	UInt3 LoadUInt3(const rapidjson::Value & value)
	{
		return UInt3{
			value[0].GetUint(),
			value[1].GetUint(),
			value[2].GetUint()
		};
	}

	UInt4 LoadUInt4(const rapidjson::Value & value)
	{
		return UInt4{
			value[0].GetUint(),
			value[1].GetUint(),
			value[2].GetUint(),
			value[3].GetUint()
		};
	}

	physx::PxQuat ToPxQuat(const Float4 & q) {
		return physx::PxQuat{ q.x, q.y, q.z, q.w };
	}

	physx::PxVec3 ToPxVec3(const Float3 & v) {
		return physx::PxVec3{ v.x, v.y, v.z };
	}

	physx::PxVec3 ToPxVec3(const physx::PxExtendedVec3 & ev)
	{
		return physx::PxVec3(static_cast<float>(ev.x), static_cast<float>(ev.y), static_cast<float>(ev.z));
	}

	physx::PxExtendedVec3 ToPxExtVec3(const Float3 & v) {
		return physx::PxExtendedVec3{ v.x, v.y, v.z };
	}

	Float3 ToFloat3(const physx::PxVec3 & v) {
		return Float3{ v.x, v.y, v.z };
	}

	Float4 ToFloat4(const physx::PxVec4 & v) {
		return Float4{ v.x, v.y, v.z, v.w };
	}

	Float4 ToFloat4(const physx::PxQuat & q) {
		return Float4{ q.x, q.y, q.z, q.w };
	}

	Vector3 ToVec3(const physx::PxVec3 & v) {
		return ToFloat3(v);
	}

	Vector3 ToVec3(const physx::PxExtendedVec3 & ev) {
		return Float3{ static_cast<float>(ev.x), static_cast<float>(ev.y), static_cast<float>(ev.z) };
	}

	Vector4 ToVec4(const physx::PxVec4 & v) {
		return ToFloat4(v);
	}

	Quaternion ToQuaternion(const physx::PxQuat & q) {
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
