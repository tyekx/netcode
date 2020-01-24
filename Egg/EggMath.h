#pragma once

#include <DirectXMath.h>
#include <physx/PxPhysicsAPI.h>

physx::PxVec3 ToPxVec3(const DirectX::XMFLOAT3 & v);

physx::PxQuat ToPxQuat(const DirectX::XMFLOAT4 & q);

DirectX::XMFLOAT3 ToFloat3(const physx::PxVec3 & v);

DirectX::XMFLOAT3A ToFloat3A(const physx::PxVec3 & v);

DirectX::XMVECTOR LoadPxVector3(const physx::PxVec3 & v);

DirectX::XMFLOAT4 ToFloat4(const physx::PxVec4 & v);

DirectX::XMFLOAT4A ToFloat4A(const physx::PxVec4 & v);

DirectX::XMFLOAT4 ToFloat4(const physx::PxQuat & q);

DirectX::XMFLOAT4A ToFloat4A(const physx::PxQuat & q);

DirectX::XMVECTOR LoadPxExtendedVec3(const physx::PxExtendedVec3 & ev);

DirectX::XMVECTOR LoadPxVector4(const physx::PxVec4 & v);

DirectX::XMVECTOR LoadPxQuat(const physx::PxQuat & q);

float RandomFloat();

float RandomFloat(float a, float b);
