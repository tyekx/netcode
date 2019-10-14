#pragma once

#include "Common.h"
#include "Mesh/Geometry.h"
#include "PsoManager.h"
#include "PipelineState.h"
#include "ConstantBuffer.hpp"

namespace Egg {


	GG_CLASS(Material) 
	public:
		static constexpr int MAX_CONSTANT_BUFFER_COUNT = 16;

	private:
		com_ptr<ID3D12PipelineState> gpso;
		Egg::PipelineState::P psoDesc;
		Egg::Mesh::Geometry::P geometry;
		int constantBufferAssoc[MAX_CONSTANT_BUFFER_COUNT];

	public:
		~Material() = default;
		Material(const Material &) = delete;

		Material(Egg::PsoManager * psoMan, Egg::Mesh::Geometry::P geom, Egg::PipelineState::P pipelineDesc) : gpso{ nullptr }, psoDesc{ pipelineDesc }, geometry{ geom } {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsoDesc;
			ZeroMemory(&gpsoDesc, sizeof(gpsoDesc));
			gpsoDesc.NumRenderTargets = 1;
			gpsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			gpsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			gpsoDesc.InputLayout = geometry->GetInputLayout();
			pipelineDesc->ApplyToDescriptor(gpsoDesc);

			gpso = psoMan->Get(gpsoDesc);

			memset(constantBufferAssoc, -1, sizeof(constantBufferAssoc));
		}

		void ConstantBufferSlot(int rootSignatureSlot, int cbvId) {
			ASSERT(MAX_CONSTANT_BUFFER_COUNT > cbvId && cbvId >= 0, "Constant buffer index must be between 0 and MAX_CONSTANT_BUFFER_COUNT");

			ASSERT(constantBufferAssoc[cbvId] == -1, "Constant buffer slot already occupied");

			constantBufferAssoc[cbvId] = rootSignatureSlot;
		}

		void ApplyPipelineState(ID3D12GraphicsCommandList * gcl) {
			gcl->SetGraphicsRootSignature(psoDesc->GetRootSignature().Get());
			gcl->SetPipelineState(gpso.Get());
		}

		template<typename T>
		void BindConstantBuffer(ID3D12GraphicsCommandList * gcl, Egg::ConstantBuffer<T> & cbuffer) {
			if(constantBufferAssoc[T::id] != -1) {
				gcl->SetGraphicsRootConstantBufferView(constantBufferAssoc[T::id], cbuffer.GetGPUVirtualAddress());
			}
		}

	GG_ENDCLASS

}
