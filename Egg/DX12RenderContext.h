#pragma once

#include "DX12Common.h"
#include "GraphicsContexts.h"
#include "DX12ResourcePool.h"
#include "DX12ConstantBufferPool.h"

namespace Egg::Graphics::DX12 {

	class GBufferCollection {
	public:
		GBuffer & GetGBuffer(int handle) { }
	};

	class RenderContext : public Egg::Graphics::IRenderContext {
		ID3D12GraphicsCommandList * gcl;
		ResourcePool * resources;
		ConstantBufferPool * cbuffers;
		std::vector<D3D12_RESOURCE_BARRIER> barriers;
		D3D12_GPU_VIRTUAL_ADDRESS streamOutput_FilledSizeLocation;
	public:

		void BeginPass() {
			ASSERT(barriers.empty(), "Barriers is not empty");
		}

		void EndPass() {
			FlushResourceBarriers();
			ResetStreamOutput();
		}

		virtual void ResourceBarrier(uint64_t handle, ResourceState before, ResourceState after) override {
			const GResource & desc = resources->GetNativeResource(handle);

			D3D12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(desc.resource, static_cast<D3D12_RESOURCE_STATES>(before), static_cast<D3D12_RESOURCE_STATES>(after));
			barriers.emplace_back(barrier);
		}

		virtual void FlushResourceBarriers() override {
			if(!barriers.empty()) {
				gcl->ResourceBarrier(barriers.size(), barriers.data());
			}

			barriers.clear();
		}

		virtual void SetConstantBuffer(int slot, uint64_t cbufferHandle) override {
			gcl->SetGraphicsRootConstantBufferView(static_cast<UINT>(slot), cbuffers->GetNativeHandle(cbufferHandle));
		}

		virtual void SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)  override {
			// not making the handle public protects the user to overwrite data before actual rendering
			uint64_t cbufferHandle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);

			cbuffers->CopyData(cbufferHandle, srcData, srcDataSizeInBytes);

			gcl->SetGraphicsRootConstantBufferView(static_cast<UINT>(slot), cbuffers->GetNativeHandle(cbufferHandle));
		}

		virtual void SetPrimitiveTopology(PrimitiveTopology topology) override {
			gcl->IASetPrimitiveTopology(static_cast<D3D12_PRIMITIVE_TOPOLOGY>(topology));
		}

		virtual void SetStreamOutputFilledSize(uint64_t handle) override {
			const GResource & gResource = resources->GetNativeResource(handle);

			streamOutput_FilledSizeLocation = gResource.address;
		}

		virtual void SetStreamOutput(uint64_t handle) override {
			ASSERT(streamOutput_FilledSizeLocation > 0, "Must call SeetStreamOutputFilledSize first");

			const GResource & gResource = resources->GetNativeResource(handle);

			D3D12_STREAM_OUTPUT_BUFFER_VIEW sobv;
			sobv.BufferFilledSizeLocation = streamOutput_FilledSizeLocation;
			sobv.BufferLocation = gResource.address;
			sobv.SizeInBytes = gResource.desc.sizeInBytes;

			gcl->SOSetTargets(0, 1, &sobv);
		}

		virtual void ResetStreamOutput() override {
			if(streamOutput_FilledSizeLocation > 0) {
				gcl->SOSetTargets(0, 0, nullptr);
				streamOutput_FilledSizeLocation = 0;
			}
		}

		virtual void Draw(uint64_t handle) override {
			GBuffer g;

			if(g.indexCount > 0) {
				gcl->IASetVertexBuffers(0, 1, &g.vertexBufferView);
				gcl->IASetIndexBuffer(&g.indexBufferView);
				gcl->DrawIndexedInstanced(g.indexCount, 1, 0, 0, 0);
			} else {
				gcl->IASetVertexBuffers(0, 1, &g.vertexBufferView);
				gcl->DrawInstanced(g.vertexCount, 1, 0, 0);
			}
		}

	};

}
