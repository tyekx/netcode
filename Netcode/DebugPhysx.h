#pragma once

#include "DebugPhysxActor.h"
#include "DebugPhysxRaycast.h"
#include <algorithm>
#include "Modules.h"
#include "BasicGeometry.h"

namespace Netcode {
	/*
	class DebugPhysx {
		Egg::PipelineStateRef debugShape_PipelineState;
		Egg::RootSignatureRef debugShape_RootSignature;
		Egg::PipelineStateRef debugRay_PipelineState;
		Egg::RootSignatureRef debugRay_RootSignature;
		uint64_t ray_VBuffer;
		uint32_t ray_VCount;

		int startRays;
		int numRays;
		constexpr static int MAX_RAY_COUNT = 32;

		Egg::Module::IGraphicsModule * graphics;
	public:

		~DebugPhysx() {
			ReleaseResources();
		}

		void CreateResources(Egg::Module::IGraphicsModule * g) {
			graphics = g;

			//raycasts = (DebugPhysxRaycast *)std::malloc(sizeof(DebugPhysxRaycast) * MAX_RAY_COUNT);
			//ZeroMemory(raycasts, sizeof(DebugPhysxRaycast) * MAX_RAY_COUNT);
			//startRays = 0;
			//numRays = 0;
			DirectX::XMFLOAT3 linePts[2];
			ray_VCount = 2;
			Egg::Graphics::BasicGeometry::CreateLine(linePts, sizeof(DirectX::XMFLOAT3));
			ray_VBuffer = g->resources->CreateVertexBuffer(sizeof(linePts), sizeof(DirectX::XMFLOAT3), Egg::Graphics::ResourceType::PERMANENT_DEFAULT, Egg::Graphics::ResourceState::COPY_DEST);

			Egg::Graphics::UploadBatch upload;
			upload.Upload(ray_VBuffer, linePts, sizeof(linePts));
			upload.ResourceBarrier(ray_VBuffer, Egg::Graphics::ResourceState::COPY_DEST, Egg::Graphics::ResourceState::VERTEX_AND_CONSTANT_BUFFER);

			{
				//debugMaterial = resMan->FromFiles(L"DebugPhysxVS.cso", L"DebugPhysxPS.cso", debugRayGeometry);


				Egg::PipelineState::P dbpso = Egg::PipelineState::Create();
				dbpso->SetRootSignature(debugPhysxRS);
				dbpso->SetVertexShader(debugPhysxVS);
				dbpso->SetPixelShader(debugPhysxPS);
				dbpso->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
				dbpso->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
				dbpso->SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

				debugMaterial = Egg::Material::Create(psoMan, layout, dbpso);
				debugMaterial->ConstantBufferSlot(DebugPhysxShapeCb::id, 0);
				debugMaterial->ConstantBufferSlot(PerObjectCb::id, 1);
				debugMaterial->ConstantBufferSlot(PerFrameCb::id, 2);
			}

			{
				Egg::PipelineState::P dbrpso = Egg::PipelineState::Create();
				dbrpso->SetRootSignature(debugPhysxRayRS);
				dbrpso->SetVertexShader(debugPhysxRayVS);
				dbrpso->SetPixelShader(debugPhysxPS);
				dbrpso->SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
				dbrpso->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
				dbrpso->SetDSVFormat(DXGI_FORMAT_D32_FLOAT);

				debugRayGeometry = Egg::BasicGeometry::CreateLine(device, DirectX::XMFLOAT3{ 0.2f, 0.3f, 0.9f });

				debugRayMaterial = Egg::Material::Create(psoMan, layout, dbrpso);
				debugRayMaterial->ConstantBufferSlot(PerRayCb::id, 0);
				debugRayMaterial->ConstantBufferSlot(PerFrameCb::id, 1);
			}

			actors.reserve(32);
		}

		Egg::DebugPhysxActor& AddActor(physx::PxRigidActor * actor) {
			actors.emplace_back(device, debugMaterial, actor);
			return actors.back();
		}

		void AfterPhysxUpdate(float dt) {

		}

		void AddRaycast(const DirectX::XMFLOAT3 & rayDir, const DirectX::XMFLOAT3 & rayStart, float length, const DirectX::XMFLOAT3 & color = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f }) {

		}

		void Draw(Egg::Graphics::IRenderContext * ctx) {
			//ctx->SetPipelineState()

		}
	};*/
}
