#include "DX12RenderContext.h"

#include <Netcode/Common.h>
#include <Netcode/HandleTypes.h>

#include "DX12SpriteFont.h"
#include "DX12Resource.h"
#include "DX12ResourcePool.h"
#include "DX12DynamicDescriptorHeap.h"
#include "DX12ConstantBufferPool.h"
#include "DX12ResourceViews.h"

namespace Netcode::Graphics::DX12 {

	BaseRenderContext::BaseRenderContext(Ptr<ResourcePool> resourcePool,
		Ptr<ConstantBufferPool> cbufferPool,
		Ptr<DynamicDescriptorHeap> dHeaps,
		Ptr<ID3D12GraphicsCommandList> cl) :
		resources{ resourcePool }, cbuffers{ cbufferPool }, descHeaps{ dHeaps }, commandList{ cl }, barriers{} {

	}

	void BaseRenderContext::UnorderedAccessBarrier(Ptr<GpuResource> handle)
	{
		ID3D12Resource * res = static_cast<DX12::Resource *>(handle)->resource.Get();

		D3D12_RESOURCE_BARRIER barrier;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.UAV.pResource = res;

		barriers.push_back(barrier);
	}

	void BaseRenderContext::ResourceBarrier(Ptr<GpuResource> handle, ResourceStates before, ResourceStates after)
	{
		ID3D12Resource * res = static_cast<DX12::Resource *>(handle)->resource.Get();

#if defined(NETCODE_DEBUG)
		const auto it = std::find_if(std::begin(barriers), std::end(barriers), [res](const D3D12_RESOURCE_BARRIER & barrier) ->bool {
			return (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) && res == barrier.Transition.pResource;
		});

		if(it != barriers.end()) {
			Log::Warn("2 resource barrier was found for the same resource, probably forgot to call FlushResourceBarriers()");
		}
#endif

		barriers.push_back(CD3DX12_RESOURCE_BARRIER::Transition(res, GetNativeState(before), GetNativeState(after)));
	}

	void BaseRenderContext::FlushResourceBarriers()
	{
		if(!barriers.empty()) {
			commandList->ResourceBarrier(static_cast<UINT>(barriers.size()), barriers.data());
			barriers.clear();
		}
	}

	GraphicsContext::GraphicsContext(	Ptr<ResourcePool> resourcePool,
										Ptr<ConstantBufferPool> cbufferPool,
										Ptr<DynamicDescriptorHeap> dHeaps,
										Ptr<ID3D12GraphicsCommandList> cl,
										const D3D12_CPU_DESCRIPTOR_HANDLE & backbuffer,
										const D3D12_CPU_DESCRIPTOR_HANDLE & backbufferDepth,
										const D3D12_VIEWPORT & viewPort,
										const D3D12_RECT & scissorRect) : BaseRenderContext{ resourcePool, cbufferPool, dHeaps, cl },
		currentlyBoundDepth{},
		currentlyBoundRenderTargets{},
		streamOutput_FilledSizeLocation{},
		currentlyBoundViewport{},
		backbuffer { backbuffer },
		backbufferDepth{ backbufferDepth },
		defaultViewport { viewPort },
		defaultScissorRect { scissorRect } {
	}

	void GraphicsContext::SetStencilReference(uint8_t stencilValue) {
		commandList->OMSetStencilRef(stencilValue);
	}

	void GraphicsContext::SetVertexBuffer(Ptr<GpuResource> handle)
	{
		DX12::Resource * rr = static_cast<DX12::Resource *>(handle);

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = rr->resource->GetGPUVirtualAddress();
		vbv.SizeInBytes = static_cast<UINT>(rr->desc.sizeInBytes);
		vbv.StrideInBytes = rr->desc.strideInBytes;

		commandList->IASetVertexBuffers(0, 1, &vbv);
	}

	void GraphicsContext::SetIndexBuffer(Ptr<GpuResource> handle)
	{
		DX12::Resource * rr = static_cast<DX12::Resource *>(handle);
		
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = rr->resource->GetGPUVirtualAddress();
		ibv.Format = rr->desc.format;
		ibv.SizeInBytes = static_cast<UINT>(rr->desc.sizeInBytes);

		commandList->IASetIndexBuffer(&ibv);
	}

	void GraphicsContext::DrawIndexed(uint32_t indexCount)
	{
		commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}

	void GraphicsContext::DrawIndexed(uint32_t indexCount, uint32_t vertexOffset)
	{
		commandList->DrawIndexedInstanced(indexCount, 1, 0, vertexOffset, 0);
	}

	void GraphicsContext::Draw(uint32_t vertexCount)
	{
		commandList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void GraphicsContext::Draw(uint32_t vertexCount, uint32_t vertexOffset)
	{
		commandList->DrawInstanced(vertexCount, 1, vertexOffset, 0);
	}

	void GraphicsContext::Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
	{
		Netcode::NotImplementedAssertion("");
	}

	void GraphicsContext::SetRootSignature(Ptr<Netcode::RootSignature> rs) {
		commandList->SetGraphicsRootSignature(reinterpret_cast<ID3D12RootSignature *>(rs->GetImplDetail()));
	}

	void GraphicsContext::SetPipelineState(Ptr<Netcode::PipelineState> pso)
	{
		commandList->SetPipelineState(reinterpret_cast<ID3D12PipelineState*>(pso->GetImplDetail()));
	}

	void GraphicsContext::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		commandList->IASetPrimitiveTopology(GetNativePrimitiveTopology(topology));
	}

	void GraphicsContext::ClearUnorderedAccessViewUint(Ptr<GpuResource> handle, const UInt4 & values)
	{
		Ptr<DX12::Resource> rr = static_cast<DX12::Resource*>(handle);

		const auto [cpuDesc, gpuDesc] = descHeaps->CreateBufferUAV(rr);

		commandList->ClearUnorderedAccessViewUint(gpuDesc, cpuDesc, rr->resource.Get(), &values.x, 0, nullptr);
	}


	void GraphicsContext::ClearDepthOnly() {
		commandList->ClearDepthStencilView(currentlyBoundDepth, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void GraphicsContext::ClearStencilOnly() {
		commandList->ClearDepthStencilView(currentlyBoundDepth, D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	}

	void GraphicsContext::ClearDepthStencil() {
		commandList->ClearDepthStencilView(currentlyBoundDepth, D3D12_CLEAR_FLAG_STENCIL | D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	}

	void GraphicsContext::ClearRenderTarget(uint8_t idx)
	{
		static const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		commandList->ClearRenderTargetView(currentlyBoundRenderTargets[idx], clearColor, 0, nullptr);
	}

	void GraphicsContext::ClearRenderTarget(uint8_t idx, const float * clearColor)
	{
		commandList->ClearRenderTargetView(currentlyBoundRenderTargets[idx], clearColor, 0, nullptr);
	}

	void GraphicsContext::SetStreamOutput(Ptr<GpuResource> handle)
	{
		DX12::Resource * rr = static_cast<DX12::Resource *>(handle);

		D3D12_STREAM_OUTPUT_BUFFER_VIEW sobv;
		sobv.BufferLocation = rr->resource->GetGPUVirtualAddress();
		sobv.BufferFilledSizeLocation = streamOutput_FilledSizeLocation;
		sobv.SizeInBytes = rr->desc.sizeInBytes;
		commandList->SOSetTargets(0, 1, &sobv);
	}

	void GraphicsContext::SetStreamOutputFilledSize(Ptr<GpuResource> handle, uint64_t byteOffset)
	{
		DX12::Resource * rr = static_cast<DX12::Resource *>(handle);

		streamOutput_FilledSizeLocation = rr->resource->GetGPUVirtualAddress() + ((byteOffset + 3ull) & ~(3ull));
	}

	void GraphicsContext::ResetStreamOutput()
	{
		if(streamOutput_FilledSizeLocation > 0) {
			commandList->SOSetTargets(0, 0, nullptr);
			streamOutput_FilledSizeLocation = 0;
		}
	}

	void GraphicsContext::SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		D3D12_VIEWPORT vp;
		vp.TopLeftX = static_cast<float>(left);
		vp.Width = static_cast<float>(right);
		vp.TopLeftY = static_cast<float>(top);
		vp.Height = static_cast<float>(bottom);
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		commandList->RSSetViewports(1, &vp);
		currentlyBoundViewport = vp;
	}

	void GraphicsContext::SetViewport(uint32_t width, uint32_t height)
	{
		SetViewport(0, width, 0, height);
	}

	void GraphicsContext::SetViewport()
	{
		commandList->RSSetViewports(1, &defaultViewport);
		currentlyBoundViewport = defaultViewport;
	}

	Float4 GraphicsContext::GetViewport()
	{
		return Float4{
			currentlyBoundViewport.TopLeftX,
			currentlyBoundViewport.TopLeftY,
			currentlyBoundViewport.Width,
			currentlyBoundViewport.Height
		};
	}

	void GraphicsContext::SetScissorRect(int32_t left, int32_t right, int32_t top, int32_t bottom)
	{
		D3D12_RECT scissorRect;
		scissorRect.left = left;
		scissorRect.right = right;
		scissorRect.top = top;
		scissorRect.bottom = bottom;

		commandList->RSSetScissorRects(1, &scissorRect);
	}

	void GraphicsContext::SetScissorRect(uint32_t width, uint32_t height)
	{
		SetScissorRect(0, static_cast<int32_t>(width), 0, static_cast<int32_t>(height));
	}

	void GraphicsContext::SetScissorRect()
	{
		commandList->RSSetScissorRects(1, &defaultScissorRect);
	}

	void GraphicsContext::SetRenderTargets(std::nullptr_t rt, Ptr<Netcode::ResourceViews> ds)
	{
		SetRenderTargets(Ptr<ResourceViews>{}, std::move(ds));
	}

	void GraphicsContext::SetRenderTargets(Ptr<Netcode::ResourceViews> rt, std::nullptr_t ds)
	{
		SetRenderTargets(std::move(rt), Ptr<ResourceViews>{});
	}

	void GraphicsContext::SetRenderTargets(Ptr<Netcode::ResourceViews> renderTargets, Ptr<Netcode::ResourceViews> depthStencil)
	{
		uint32_t numDescriptors;
		if(renderTargets != nullptr) {
			DX12::ResourceViewsImpl* rtvs = static_cast<DX12::ResourceViewsImpl*>(renderTargets);
			numDescriptors = rtvs->GetNumDescriptors();
			for(uint32_t i = 0; i < numDescriptors; ++i) {
				currentlyBoundRenderTargets[i] = rtvs->GetCpuVisibleCpuHandle(i);
			}
		} else {
			currentlyBoundRenderTargets[0] = backbuffer;
			numDescriptors = 1;
		}

		if(depthStencil != nullptr) {
			DX12::ResourceViewsImpl* dsv = static_cast<DX12::ResourceViewsImpl*>(depthStencil);
			currentlyBoundDepth = dsv->GetCpuVisibleCpuHandle(0);
		} else {
			currentlyBoundDepth = backbufferDepth;
		}

		commandList->OMSetRenderTargets(numDescriptors, currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetRenderTargets(std::initializer_list<Ptr<GpuResource>> handles, Ptr<GpuResource> depthStencil)
	{
		uint8_t k = 0;
		for(const Ptr<GpuResource> & rtHandle : handles) {
			ASSERT(rtHandle != nullptr, "SetRenderTargets(...): handles array must not contain null handle");
			Ptr<DX12::Resource> rr = static_cast<DX12::Resource*>(rtHandle);

			currentlyBoundRenderTargets[k++] = descHeaps->CreateRTV(rr);
		}

		if(depthStencil == nullptr) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			Ptr<DX12::Resource> rr = static_cast<DX12::Resource*>(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(rr);
		}

		commandList->OMSetRenderTargets(static_cast<uint32_t>(handles.size()), currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetRenderTargets(std::nullptr_t rt, std::nullptr_t ds)
	{
		SetRenderTargets(Ptr<GpuResource>{}, Ptr<GpuResource>{});
	}

	void GraphicsContext::SetRenderTargets(std::nullptr_t rt, Ptr<GpuResource> ds)
	{
		SetRenderTargets(Ptr<GpuResource>{}, std::move(ds));
	}

	void GraphicsContext::SetRenderTargets(Ptr<GpuResource> rt, std::nullptr_t ds)
	{
		SetRenderTargets(std::move(rt), Ptr<GpuResource>{});
	}

	void GraphicsContext::SetRenderTargets(Ptr<GpuResource> handle, Ptr<GpuResource> depthStencil)
	{
		if(handle == nullptr) {
			currentlyBoundRenderTargets[0] = backbuffer;
		} else {
			Ptr<DX12::Resource> rr = static_cast<DX12::Resource*>(handle);
			currentlyBoundRenderTargets[0] = descHeaps->CreateRTV(rr);
		}

		if(depthStencil == nullptr) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			Ptr<DX12::Resource> rr = static_cast<DX12::Resource*>(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(rr);
		}

		commandList->OMSetRenderTargets(1, currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetShaderResources(int slot, std::initializer_list<Ptr<GpuResource>> shaderResourceHandles) {
		

		D3D12_GPU_DESCRIPTOR_HANDLE descriptor;
		descriptor.ptr = 0;
		
		for(const Ptr<GpuResource> & i : shaderResourceHandles) {
			Ptr<DX12::Resource> rr = static_cast<DX12::Resource*>(i);
			if(descriptor.ptr == 0) {
				descriptor = descHeaps->CreateSRV(rr);
			} else descHeaps->CreateSRV(rr);
		}

		commandList->SetGraphicsRootDescriptorTable(slot, descriptor);
	}

	void GraphicsContext::SetShaderResources(int slot, Ptr<Netcode::ResourceViews> resourceView)
	{
		SetShaderResources(slot, std::move(resourceView), 0);
	}

	void GraphicsContext::SetShaderResources(int slot, Ptr<Netcode::ResourceViews> resourceView, int descriptorOffset)
	{
		if(resourceView == nullptr) {
			return;
		}
		
		DX12::ResourceViewsImpl* srv = static_cast<DX12::ResourceViewsImpl*>(resourceView);

		commandList->SetGraphicsRootDescriptorTable(slot, srv->GetGpuHandle(descriptorOffset));
	}

	void GraphicsContext::SetShaderResources(int slot, Ptr<GpuResource> nonTextureResource) {
		DX12::Resource* resource = static_cast<DX12::Resource*>(nonTextureResource);

		commandList->SetGraphicsRootShaderResourceView(slot, resource->resource->GetGPUVirtualAddress());
	}
	
	void GraphicsContext::SetRootConstants(int slot, const void * srcData, uint32_t numConstants) {
		commandList->SetGraphicsRoot32BitConstants(slot, numConstants, srcData, 0);
	}

	void GraphicsContext::SetRootConstants(int slot, const void * srcData, uint32_t num32bitConstants, uint32_t offsetIn32BitConstants)
	{
		commandList->SetGraphicsRoot32BitConstants(slot, num32bitConstants, srcData, offsetIn32BitConstants);
	}

	void GraphicsContext::SetConstantBuffer(int slot, Ptr<GpuResource> cbufferHandle)
	{
		DX12::Resource * rr = static_cast<DX12::Resource *>(cbufferHandle);

		commandList->SetGraphicsRootConstantBufferView(slot, rr->resource->GetGPUVirtualAddress());
	}

	void GraphicsContext::SetConstants(int slot, uint64_t constantHandle)
	{
		commandList->SetGraphicsRootConstantBufferView(slot, cbuffers->GetNativeHandle(constantHandle));
	}

	uint64_t GraphicsContext::SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)
	{
		uint64_t handle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);
		cbuffers->CopyData(handle, srcData, srcDataSizeInBytes);

		commandList->SetGraphicsRootConstantBufferView(slot, cbuffers->GetNativeHandle(handle));

		return handle;
	}

	void GraphicsContext::CopyBufferRegion(Ptr<GpuResource> dstResource, Ptr<GpuResource> srcResource, size_t sizeInBytes)
	{
		CopyBufferRegion(std::move(dstResource), 0, std::move(srcResource), 0, sizeInBytes);
	}

	void GraphicsContext::CopyBufferRegion(Ptr<GpuResource> dstResource, size_t dstOffset, Ptr<GpuResource> srcResource, size_t srcOffset, size_t sizeInBytes)
	{
		DX12::Resource * rr0 = static_cast<DX12::Resource *>(dstResource);
		DX12::Resource * rr1 = static_cast<DX12::Resource *>(srcResource);

		commandList->CopyBufferRegion(rr0->resource.Get(), dstOffset, rr1->resource.Get(), srcOffset, sizeInBytes);
	}

	void GraphicsContext::BeginPass()
	{
		descHeaps->SetDescriptorHeaps(commandList);
	}

	void GraphicsContext::EndPass()
	{
		FlushResourceBarriers();
		ResetStreamOutput();
	}

	void ComputeContext::SetRootSignature(Ptr<Netcode::RootSignature> rs)
	{
		commandList->SetComputeRootSignature(reinterpret_cast<ID3D12RootSignature *>(rs->GetImplDetail()));
	}

	void ComputeContext::SetPipelineState(Ptr<Netcode::PipelineState> pso)
	{
		commandList->SetPipelineState(reinterpret_cast<ID3D12PipelineState *>(pso->GetImplDetail()));
	}

	void ComputeContext::SetVertexBuffer(Ptr<GpuResource> handle)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetIndexBuffer(Ptr<GpuResource> handle)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::DrawIndexed(uint32_t indexCount)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::DrawIndexed(uint32_t indexCount, uint32_t vertexOffset)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::Draw(uint32_t vertexCount)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::Draw(uint32_t vertexCount, uint32_t vertexOffset)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
	{
		commandList->Dispatch(threadGroupX, threadGroupY, threadGroupZ);
	}

	void ComputeContext::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::ClearUnorderedAccessViewUint(Ptr<GpuResource> handle, const UInt4 & values)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::ClearRenderTarget(uint8_t idx)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::ClearRenderTarget(uint8_t idx, const float * clearColor)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::ClearDepthOnly()
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::ClearStencilOnly()
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::ClearDepthStencil()
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetStreamOutput(Ptr<GpuResource> handle)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetStreamOutputFilledSize(Ptr<GpuResource> handle, uint64_t byteOffset)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::ResetStreamOutput()
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(std::initializer_list<Ptr<GpuResource>> handles, Ptr<GpuResource> depthStencil)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetStencilReference(uint8_t stencilValue)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetViewport(uint32_t width, uint32_t height)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetViewport()
	{
		Netcode::NotImplementedAssertion("");
	}

	Float4 ComputeContext::GetViewport()
	{
		Netcode::NotImplementedAssertion("");
		return Float4::Zero;
	}

	void ComputeContext::SetScissorRect(int32_t left, int32_t right, int32_t top, int32_t bottom)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetScissorRect(uint32_t width, uint32_t height)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetScissorRect()
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(Ptr<GpuResource> renderTarget, Ptr<GpuResource> depthStencil)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(std::nullptr_t rt, std::nullptr_t ds)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(std::nullptr_t rt, Ptr<Netcode::ResourceViews> ds)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(Ptr<Netcode::ResourceViews> rt, std::nullptr_t ds)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(Ptr<Netcode::ResourceViews> renderTargets, Ptr<Netcode::ResourceViews> depthStencil)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(std::nullptr_t rt, Ptr<GpuResource> ds)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRenderTargets(Ptr<GpuResource> rt, std::nullptr_t ds)
	{
		Netcode::NotImplementedAssertion("");
	}

	void ComputeContext::SetRootConstants(int slot, const void * srcData, uint32_t numConstants)
	{
		commandList->SetComputeRoot32BitConstants(slot, numConstants, srcData, 0);
	}

	void ComputeContext::SetRootConstants(int slot, const void * srcData, uint32_t num32bitConstants, uint32_t offsetIn32BitConstants)
	{
		commandList->SetComputeRoot32BitConstants(slot, num32bitConstants, srcData, offsetIn32BitConstants);
	}

	void ComputeContext::SetShaderResources(int slot, std::initializer_list<Ptr<GpuResource>> shaderResourceHandles) {
		D3D12_GPU_DESCRIPTOR_HANDLE descriptor;
		descriptor.ptr = 0;

		for(const Ptr<GpuResource> & i : shaderResourceHandles) {
			Ptr<DX12::Resource> rr = static_cast<DX12::Resource*>(i);
			if(descriptor.ptr == 0) {
				descriptor = descHeaps->CreateSRV(rr);
			} else descHeaps->CreateSRV(rr);
		}

		commandList->SetComputeRootDescriptorTable(slot, descriptor);
	}

	void ComputeContext::SetShaderResources(int slot, Ptr<Netcode::ResourceViews> resourceView)
	{
		SetShaderResources(slot, resourceView, 0);
	}

	void ComputeContext::SetShaderResources(int slot, Ptr<Netcode::ResourceViews> resourceView, int descriptorOffset)
	{
		DX12::ResourceViewsImpl* srv = static_cast<DX12::ResourceViewsImpl*>(resourceView);

		commandList->SetComputeRootDescriptorTable(slot, srv->GetGpuHandle(descriptorOffset));
	}

	void ComputeContext::SetShaderResources(int slot, Ptr<GpuResource> nonTextureResource) {
		DX12::Resource* resource = static_cast<DX12::Resource*>(nonTextureResource);

		commandList->SetComputeRootShaderResourceView(slot, resource->resource->GetGPUVirtualAddress());
	}

	void ComputeContext::SetConstantBuffer(int slot, Ptr<GpuResource> cbufferHandle)
	{
		DX12::Resource* rr = static_cast<DX12::Resource*>(cbufferHandle);

		commandList->SetComputeRootConstantBufferView(slot, rr->resource->GetGPUVirtualAddress());
	}

	void ComputeContext::SetConstants(int slot, uint64_t constantHandle)
	{
		commandList->SetComputeRootConstantBufferView(slot, cbuffers->GetNativeHandle(constantHandle));
	}

	uint64_t ComputeContext::SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)
	{
		uint64_t handle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);
		cbuffers->CopyData(handle, srcData, srcDataSizeInBytes);

		commandList->SetComputeRootConstantBufferView(slot, cbuffers->GetNativeHandle(handle));

		return handle;
	}

	void ComputeContext::CopyBufferRegion(Ptr<GpuResource> dstResource, Ptr<GpuResource> srcResource, size_t sizeInBytes)
	{
		CopyBufferRegion(dstResource, 0, srcResource, 0, sizeInBytes);
	}

	void ComputeContext::CopyBufferRegion(Ptr<GpuResource> dstResource, size_t dstOffset, Ptr<GpuResource> srcResource, size_t srcOffset, size_t sizeInBytes)
	{
		DX12::Resource* rr0 = static_cast<DX12::Resource*>(dstResource);
		DX12::Resource* rr1 = static_cast<DX12::Resource*>(srcResource);

		commandList->CopyBufferRegion(rr0->resource.Get(), dstOffset, rr1->resource.Get(), srcOffset, sizeInBytes);
	}

	void ComputeContext::BeginPass()
	{
		descHeaps->SetDescriptorHeaps(commandList);
	}

	void ComputeContext::EndPass()
	{
		FlushResourceBarriers();
	}

}
