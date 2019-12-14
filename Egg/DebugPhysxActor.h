#pragma once

#include "BasicGeometry.h"
#include <array>


namespace Egg {
	/*
	class DebugPhysxActor {
		physx::PxRigidActor * actorReference;
		std::vector<Egg::Graphics::Geometry> geometries;
		std::vector<Egg::Mesh> meshes;
		ConstantBuffer<PerObjectCb> perObject;
		Egg::Material debugMaterial;
		std::array<physx::PxShape*, 16> shapes;
		ConstantBufferArray<DebugPhysxShapeCb, 16> perShapeCb;

	public:

		void SetOffset(unsigned int shapeId, DirectX::XMMATRIX offsetMatrix) {
			DirectX::XMStoreFloat4x4A(&perShapeCb[shapeId].offset, offsetMatrix);
			perShapeCb.Upload();
		}

		void UploadResources(ID3D12GraphicsCommandList * gcl) {
			for(auto & i : geometries) {
				//i.vertexBuffer->UploadResources(gcl);
			}
		}

		void ReleaseUploadResources() {
			for(auto & i : geometries) {
				i.vertexBuffer->ReleaseUploadResources();
			}
		}

		DebugPhysxActor(ID3D12Device * device, const Egg::Material & mat, physx::PxRigidActor * actorRef);

		void AfterPhysxUpdate();

		void Draw(ID3D12GraphicsCommandList * cl);
	};*/
}
