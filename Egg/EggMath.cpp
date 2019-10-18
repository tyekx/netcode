#include "EggMath.h"

physx::PxVec3 ToPxVec3(const DirectX::XMFLOAT3 & v) {
	return physx::PxVec3{ v.x, v.y, v.z };
}

DirectX::XMFLOAT3 ToFloat3(const physx::PxVec3 & v) {
	return DirectX::XMFLOAT3{ v.x, v.y, v.z };
}

DirectX::XMFLOAT3A ToFloat3A(const physx::PxVec3 & v) {
	return DirectX::XMFLOAT3A{ v.x, v.y, v.z };
}

DirectX::XMVECTOR LoadPxVector3(const physx::PxVec3 & v) {
	auto f3a = ToFloat3A(v);
	return DirectX::XMLoadFloat3A(&f3a);
}

DirectX::XMFLOAT4 ToFloat4(const physx::PxVec4 & v) {
	return DirectX::XMFLOAT4{ v.x, v.y, v.z, v.w };
}

DirectX::XMFLOAT4A ToFloat4A(const physx::PxVec4 & v) {
	return DirectX::XMFLOAT4A{ v.x, v.y, v.z, v.w };
}

DirectX::XMFLOAT4 ToFloat4(const physx::PxQuat & q) {
	return DirectX::XMFLOAT4{ q.x, q.y, q.z, q.w };
}

DirectX::XMFLOAT4A ToFloat4A(const physx::PxQuat & q) {
	return DirectX::XMFLOAT4A{ q.x, q.y, q.z, q.w };
}


DirectX::XMVECTOR LoadPxExtendedVec3(const physx::PxExtendedVec3 & ev) {
	DirectX::XMFLOAT3A f3a{ (float)ev.x, (float)ev.y, (float)ev.z };
	return DirectX::XMLoadFloat3A(&f3a);
}

DirectX::XMVECTOR LoadPxVector4(const physx::PxVec4 & v) {
	auto f4a = ToFloat4A(v);
	return DirectX::XMLoadFloat4A(&f4a);
}

DirectX::XMVECTOR LoadPxQuat(const physx::PxQuat & q) {
	auto f4a = ToFloat4A(q);
	return DirectX::XMLoadFloat4A(&f4a);
}
