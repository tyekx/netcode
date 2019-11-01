#pragma once

#include "BasicGeometry.h"
#include "PhysxSystem.h"
#include <array>

namespace Egg {


	__declspec(align(16)) struct DebugPhysxShapeCb {
		constexpr static int id = 15;

		DirectX::XMFLOAT4X4A local;
		DirectX::XMFLOAT4X4A offset;
	};

	class DebugPhysxActor {
		physx::PxRigidActor * actorReference;
		std::vector<Egg::Mesh> geometry;
		ConstantBuffer<PerObjectCb> perObject;
		Egg::Material* debugMaterial;
		std::array<physx::PxShape*, 16> shapes;
		ConstantBufferArray<DebugPhysxShapeCb, 16> perShapeCb;

	public:

		void SetOffset(unsigned int shapeId, DirectX::XMMATRIX offsetMatrix) {
			DirectX::XMStoreFloat4x4A(&perShapeCb[shapeId].offset, offsetMatrix);
			perShapeCb.Upload();
		}

		DebugPhysxActor(ID3D12Device * device, Egg::Material * mat, physx::PxRigidActor * actorRef);

		void AfterPhysxUpdate();

		void Draw(ID3D12GraphicsCommandList * cl);
	};
}
