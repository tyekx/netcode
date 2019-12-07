#pragma once

#include "DebugPhysxActor.h"
#include "DebugPhysxRaycast.h"
#include <algorithm>

namespace Egg {
	/*
	class DebugPhysx {
		Egg::Material debugMaterial;
		Egg::Material debugRayMaterial;
		Egg::Graphics::Geometry debugRayGeometry;

		int startRays;
		int numRays;
		constexpr static int MAX_RAY_COUNT = 32;

		DebugPhysxRaycast* raycasts;
		std::vector<DebugPhysxActor> actors;
		ID3D12Device * device;

	public:

		~DebugPhysx() {
			ReleaseResources();
		}

		void UploadResources(ID3D12GraphicsCommandList * gcl) {
			for(auto & i : actors) {
				i.UploadResources(gcl);
			}
		}

		void ReleaseUploadResources() {
			for(auto & i : actors) {
				i.ReleaseUploadResources();
			}
		}

		void CreateResources(ID3D12Device * dev) {
			device = dev;
			raycasts = (DebugPhysxRaycast *)std::malloc(sizeof(DebugPhysxRaycast) * MAX_RAY_COUNT);
			ZeroMemory(raycasts, sizeof(DebugPhysxRaycast) * MAX_RAY_COUNT);
			startRays = 0;
			numRays = 0;

			debugRayGeometry = BasicGeometry::CreateLine(device, DirectX::XMFLOAT3{ 0.2f, 0.3f, 0.9f });

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

		void ReleaseResources() {
			actors.clear();

			for(int i = 0; i < numRays; ++i) {
				raycasts[(startRays + i) % MAX_RAY_COUNT].ReleaseResources();
			}

			std::free(raycasts);
			numRays = 0;
			startRays = 0;

			//debugMaterial.reset();
			//debugRayMaterial.reset();
			//debugRayGeometry.reset();
		}

		Egg::DebugPhysxActor& AddActor(physx::PxRigidActor * actor) {
			actors.emplace_back(device, debugMaterial, actor);
			return actors.back();
		}

		void RemoveActor(physx::PxActor * actor) {
			//@todo
		}

		void AfterPhysxUpdate(float dt) {
			for(auto & i : actors) {
				i.AfterPhysxUpdate();
			}

			for(int i = 0; i < numRays; ++i) {
				raycasts[(startRays + i) % MAX_RAY_COUNT].Update(dt);
			}

			for(int i = 0; i < numRays; ++i) {
				if(raycasts[(startRays + i) % MAX_RAY_COUNT].IsAlive()) {
					break;
				} else {
					raycasts[(startRays + i) % MAX_RAY_COUNT].~DebugPhysxRaycast();
					--numRays;
					startRays = (startRays + 1) % MAX_RAY_COUNT;
				}
			}
		}

		void AddRaycast(const DirectX::XMFLOAT3 & rayDir, const DirectX::XMFLOAT3 & rayStart, float length, const DirectX::XMFLOAT3 & color = DirectX::XMFLOAT3{ 0.0f, 0.0f, 1.0f }) {

			if(numRays < MAX_RAY_COUNT) {
				new (raycasts + ((startRays + numRays) % MAX_RAY_COUNT)) DebugPhysxRaycast(device, debugRayMaterial.get(), debugRayGeometry.get(),
																		 10.0f, rayDir, rayStart, color, length);
				++numRays;
			}

		}

		void Draw(ID3D12GraphicsCommandList * gcl, D3D12_GPU_VIRTUAL_ADDRESS perFrameCb) {
			debugMaterial.SetPipelineState(gcl);
			debugMaterial.BindConstantBuffer(gcl, PerFrameCb::id, perFrameCb);

			for(auto & i : actors) {
				i.Draw(gcl);
			}

			//debugRayMaterial.SetPipelineState(gcl);
			//debugRayMaterial.BindConstantBuffer(gcl, PerFrameCb::id, perFrameCb);

			//for(int i = 0; i < numRays; ++i) {
			//	raycasts[(startRays + i) % MAX_RAY_COUNT].Draw(gcl);
			//}
		}

	};
	*/
}
