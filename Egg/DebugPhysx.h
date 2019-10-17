#pragma once

#include "BasicGeometry.h"
#include "PhysxSystem.h"
#include "Shader.h"

namespace Egg {
	
	class DebugPhysx {
		physx::PxRigidActor * actorReference;
		Egg::Mesh::Geometry::P geometry;
		ConstantBuffer<PerObjectCb> perObject;
		static Egg::Material::P debugMaterial;

	public:
		DirectX::XMFLOAT4X4A offset;

		DebugPhysx(ID3D12Device* device, PsoManager* psoMan, physx::PxRigidActor * actorRef) : actorReference{ actorRef } {
			DirectX::XMMATRIX identity = DirectX::XMMatrixIdentity();
			DirectX::XMStoreFloat4x4A(&offset, identity);

			perObject.CreateResources(device);

			physx::PxU32 numShapes = actorRef->getNbShapes();

			physx::PxShape * shape;

			actorRef->getShapes(&shape, 1);
			physx::PxGeometryType::Enum type = shape->getGeometryType();

			if(type == physx::PxGeometryType::eCAPSULE) {
				physx::PxCapsuleGeometry capsule;
				shape->getCapsuleGeometry(capsule);
				geometry = BasicGeometry::CreateCapsuleSilhouette(device, capsule.halfHeight * 2.0f, capsule.radius, DirectX::XMFLOAT3{ 1.0f, 0.0f, 0.0f });
			}

			if(type == physx::PxGeometryType::ePLANE) {
				physx::PxPlaneGeometry plane;
				shape->getPlaneGeometry(plane);
				geometry = BasicGeometry::CreatePlaneWireframe(device, 50, DirectX::XMFLOAT3{ 0.5f, 0.5f, 0.5f });
			}

			if(debugMaterial == nullptr) {
				com_ptr<ID3DBlob> debugPhysxVS = Egg::Shader::LoadCso(L"DebugPhysxVS.cso");
				com_ptr<ID3DBlob> debugPhysxPS = Egg::Shader::LoadCso(L"DebugPhysxPS.cso");
				com_ptr<ID3D12RootSignature> debugPhysxRS = Egg::Shader::LoadRootSignature(device, debugPhysxVS.Get());

				Egg::PipelineState::P dbpso = Egg::PipelineState::Create();
				dbpso->SetRootSignature(debugPhysxRS);
				dbpso->SetVertexShader(debugPhysxVS);
				dbpso->SetPixelShader(debugPhysxPS);
				dbpso->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
				dbpso->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);

				debugMaterial = Egg::Material::Create(psoMan, geometry, dbpso);
				debugMaterial->ConstantBufferSlot(0, PerFrameCb::id);
				debugMaterial->ConstantBufferSlot(1, PerObjectCb::id);
			}
		}

		void AfterPhysxUpdate() {
			physx::PxTransform transform = actorReference->getGlobalPose();
			//DirectX::XMFLOAT4A quatSource{ transform.q.x, transform.q.y, transform.q.z, transform.q.w };

			DirectX::XMMATRIX translationMat = DirectX::XMMatrixTranslation(transform.p.x, transform.p.y, transform.p.z);
			//DirectX::XMVECTOR quat = DirectX::XMLoadFloat4(&quatSource);
			//DirectX::XMMATRIX rotationMat = DirectX::XMMatrixRotationQuaternion(quat);
			DirectX::XMMATRIX offsetMat = DirectX::XMLoadFloat4x4A(&offset);
			DirectX::XMMATRIX model = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(offsetMat, translationMat));
			DirectX::XMStoreFloat4x4A(&perObject->Model, model);
			perObject.Upload();
		}

		void Draw(ID3D12GraphicsCommandList * cl, ConstantBuffer<PerFrameCb> & pfcb) {
			debugMaterial->ApplyPipelineState(cl);
			debugMaterial->BindConstantBuffer(cl, perObject);
			debugMaterial->BindConstantBuffer(cl, pfcb);

			geometry->Draw(cl);
		}
	};


}
