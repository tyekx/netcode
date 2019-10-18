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
		int constantBufferAssoc[MAX_CONSTANT_BUFFER_COUNT];

	public:
		~Material() = default;
		Material(const Material &) = delete;

		Material(Egg::PsoManager * psoMan, Egg::Mesh::Geometry::P geom, Egg::PipelineState::P pipelineDesc);

		Material(Egg::PsoManager * psoMan, const D3D12_INPUT_LAYOUT_DESC & layout, Egg::PipelineState::P pipelineDesc);

		void ConstantBufferSlot(int cbvId, int rootSignatureSlot);

		void ApplyPipelineState(ID3D12GraphicsCommandList * gcl);

		template<typename T>
		void BindConstantBuffer(ID3D12GraphicsCommandList * gcl, Egg::ConstantBuffer<T> & cbuffer) {
			if(constantBufferAssoc[T::id] != -1) {
				gcl->SetGraphicsRootConstantBufferView(constantBufferAssoc[T::id], cbuffer.GetGPUVirtualAddress());
			} else {
				OutputDebugString("Cbuffer was not found\r\n");
			}
		}

		void BindConstantBuffer(ID3D12GraphicsCommandList * gcl, unsigned int cbufferTypeId, D3D12_GPU_VIRTUAL_ADDRESS addr) {
			if(constantBufferAssoc[cbufferTypeId] != -1) {
				gcl->SetGraphicsRootConstantBufferView(constantBufferAssoc[cbufferTypeId], addr);
			} else {
				OutputDebugString("Cbuffer was not found\r\n");
			}
		}

	GG_ENDCLASS

}
