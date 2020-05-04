#include "DX12RenderContext.h"
#include "DX12SpriteFont.h"
#include "DX12Resource.h"

namespace Netcode::Graphics::DX12 {

	BaseRenderContext::BaseRenderContext(ResourcePool * resourcePool, ConstantBufferPool * cbufferPool, DynamicDescriptorHeap * dHeaps, com_ptr<ID3D12GraphicsCommandList> cl) :
		resources{ resourcePool }, cbuffers{ cbufferPool }, descHeaps{ dHeaps }, commandList{ std::move(cl) }, barriers{}
	{

	}

	void BaseRenderContext::ResourceBarrier(GpuResourceRef handle, ResourceState before, ResourceState after)
	{
		ID3D12Resource * res = std::dynamic_pointer_cast<DX12Resource>(handle)->resource.Get();

#if defined(EGG_DEBUG)
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

	GraphicsContext::GraphicsContext(
		ResourcePool * resourcePool,
		ConstantBufferPool * cbpool,
		DynamicDescriptorHeap * dheaps,
		com_ptr<ID3D12GraphicsCommandList> commandListRef,
		const D3D12_CPU_DESCRIPTOR_HANDLE & backbuffer,
		const D3D12_CPU_DESCRIPTOR_HANDLE & backbufferDepth,
		const D3D12_VIEWPORT & viewPort,
		const D3D12_RECT & scissorRect
	) : BaseRenderContext(resourcePool, cbpool, dheaps, std::move(commandListRef)),
		currentlyBoundDepth{},
		currentlyBoundRenderTargets{},
		streamOutput_FilledSizeLocation{},
		backbuffer { backbuffer },
		backbufferDepth{ backbufferDepth },
		defaultViewport { viewPort },
		defaultScissorRect { scissorRect }
	{
	}

	void GraphicsContext::SetStencilReference(uint8_t stencilValue) {
		commandList->OMSetStencilRef(stencilValue);
	}

	void GraphicsContext::SetVertexBuffer(GpuResourceRef handle)
	{
		DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(handle);

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = rr->resource->GetGPUVirtualAddress();
		vbv.SizeInBytes = static_cast<UINT>(rr->desc.sizeInBytes);
		vbv.StrideInBytes = rr->desc.strideInBytes;

		commandList->IASetVertexBuffers(0, 1, &vbv);
	}

	void GraphicsContext::SetIndexBuffer(GpuResourceRef handle)
	{
		DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(handle);
		
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
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void GraphicsContext::SetRootSignature(RootSignatureRef rs) {
		commandList->SetGraphicsRootSignature(reinterpret_cast<ID3D12RootSignature *>(rs->GetImplDetail()));
	}

	void GraphicsContext::SetPipelineState(PipelineStateRef pso)
	{
		commandList->SetPipelineState(reinterpret_cast<ID3D12PipelineState*>(pso->GetImplDetail()));
	}

	void GraphicsContext::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		commandList->IASetPrimitiveTopology(GetNativePrimitiveTopology(topology));
	}

	void GraphicsContext::ClearUnorderedAccessViewUint(GpuResourceRef handle, const DirectX::XMUINT4 & values)
	{
		DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(handle);

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

	void GraphicsContext::SetStreamOutput(GpuResourceRef handle)
	{
		DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(handle);

		D3D12_STREAM_OUTPUT_BUFFER_VIEW sobv;
		sobv.BufferLocation = rr->resource->GetGPUVirtualAddress();
		sobv.BufferFilledSizeLocation = streamOutput_FilledSizeLocation;
		sobv.SizeInBytes = rr->desc.sizeInBytes;
		commandList->SOSetTargets(0, 1, &sobv);
	}

	void GraphicsContext::SetStreamOutputFilledSize(GpuResourceRef handle, uint64_t byteOffset)
	{
		DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(handle);

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
	}

	void GraphicsContext::SetViewport(uint32_t width, uint32_t height)
	{
		SetViewport(0, width, 0, height);
	}

	void GraphicsContext::SetViewport()
	{
		commandList->RSSetViewports(1, &defaultViewport);
	}

	void GraphicsContext::SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
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
		SetScissorRect(0, width, 0, height);
	}

	void GraphicsContext::SetScissorRect()
	{
		commandList->RSSetScissorRects(1, &defaultScissorRect);
	}

	void GraphicsContext::SetRenderTargets(std::nullptr_t rt, ResourceViewsRef ds)
	{
		SetRenderTargets(ResourceViewsRef{}, std::move(ds));
	}

	void GraphicsContext::SetRenderTargets(ResourceViewsRef rt, std::nullptr_t ds)
	{
		SetRenderTargets(std::move(rt), ResourceViewsRef{});
	}

	void GraphicsContext::SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil)
	{
		uint32_t numDescriptors;
		if(renderTargets != nullptr) {
			DX12ResourceViewsRef rtvs = std::dynamic_pointer_cast<DX12ResourceViews>(renderTargets);
			numDescriptors = rtvs->GetNumDescriptors();
			for(uint32_t i = 0; i < numDescriptors; ++i) {
				currentlyBoundRenderTargets[i] = rtvs->GetCpuVisibleCpuHandle(i);
			}
		} else {
			currentlyBoundRenderTargets[0] = backbuffer;
			numDescriptors = 1;
		}

		if(depthStencil != nullptr) {
			DX12ResourceViewsRef dsv = std::dynamic_pointer_cast<DX12ResourceViews>(depthStencil);
			currentlyBoundDepth = dsv->GetCpuVisibleCpuHandle(0);
		} else {
			currentlyBoundDepth = backbufferDepth;
		}

		commandList->OMSetRenderTargets(numDescriptors, currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetRenderTargets(std::initializer_list<GpuResourceRef> handles, GpuResourceRef depthStencil)
	{
		uint8_t k = 0;
		for(const GpuResourceRef & rtHandle : handles) {
			ASSERT(rtHandle != nullptr, "SetRenderTargets(...): handles array must not contain null handle");
			DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(rtHandle);

			currentlyBoundRenderTargets[k++] = descHeaps->CreateRTV(rr);
		}

		if(depthStencil == nullptr) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(rr);
		}

		commandList->OMSetRenderTargets(static_cast<uint32_t>(handles.size()), currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetRenderTargets(std::nullptr_t rt, std::nullptr_t ds)
	{
		SetRenderTargets(GpuResourceRef{}, GpuResourceRef{});
	}

	void GraphicsContext::SetRenderTargets(std::nullptr_t rt, GpuResourceRef ds)
	{
		SetRenderTargets(GpuResourceRef{}, std::move(ds));
	}

	void GraphicsContext::SetRenderTargets(GpuResourceRef rt, std::nullptr_t ds)
	{
		SetRenderTargets(std::move(rt), GpuResourceRef{});
	}

	void GraphicsContext::SetRenderTargets(GpuResourceRef handle, GpuResourceRef depthStencil)
	{
		if(handle == nullptr) {
			currentlyBoundRenderTargets[0] = backbuffer;
		} else {
			DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(handle);
			currentlyBoundRenderTargets[0] = descHeaps->CreateRTV(rr);
		}

		if(depthStencil == nullptr) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(rr);
		}

		commandList->OMSetRenderTargets(1, currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetShaderResources(int slot, std::initializer_list<GpuResourceRef> shaderResourceHandles) {
		

		D3D12_GPU_DESCRIPTOR_HANDLE descriptor;
		descriptor.ptr = 0;
		
		for(const GpuResourceRef & i : shaderResourceHandles) {
			DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(i);
			if(descriptor.ptr == 0) {
				descriptor = descHeaps->CreateSRV(rr);
			} else descHeaps->CreateSRV(rr);
		}

		commandList->SetGraphicsRootDescriptorTable(slot, descriptor);
	}

	void GraphicsContext::SetShaderResources(int slot, ResourceViewsRef resourceView)
	{
		SetShaderResources(slot, resourceView, 0);
	}

	void GraphicsContext::SetShaderResources(int slot, ResourceViewsRef resourceView, int descriptorOffset)
	{
		DX12ResourceViewsRef srv = std::dynamic_pointer_cast<DX12ResourceViews>(resourceView);

		commandList->SetGraphicsRootDescriptorTable(slot, srv->GetGpuHandle(descriptorOffset));
	}
	
	void GraphicsContext::SetRootConstants(int slot, const void * srcData, uint32_t numConstants) {
		commandList->SetGraphicsRoot32BitConstants(slot, numConstants, srcData, 0);
	}

	void GraphicsContext::SetConstantBuffer(int slot, GpuResourceRef cbufferHandle)
	{
		DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(cbufferHandle);

		commandList->SetGraphicsRootConstantBufferView(slot, rr->resource->GetGPUVirtualAddress());
	}

	void GraphicsContext::SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)
	{
		uint64_t handle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);
		cbuffers->CopyData(handle, srcData, srcDataSizeInBytes);

		commandList->SetGraphicsRootConstantBufferView(slot, cbuffers->GetNativeHandle(handle));
	}

	void GraphicsContext::CopyBufferRegion(GpuResourceRef dstResource, GpuResourceRef srcResource, size_t sizeInBytes)
	{
		CopyBufferRegion(dstResource, 0, srcResource, 0, sizeInBytes);
	}

	void GraphicsContext::CopyBufferRegion(GpuResourceRef dstResource, size_t dstOffset, GpuResourceRef srcResource, size_t srcOffset, size_t sizeInBytes)
	{
		DX12ResourceRef rr0 = std::dynamic_pointer_cast<DX12Resource>(dstResource);
		DX12ResourceRef rr1 = std::dynamic_pointer_cast<DX12Resource>(srcResource);

		commandList->CopyBufferRegion(rr0->resource.Get(), dstOffset, rr1->resource.Get(), srcOffset, sizeInBytes);
	}

	void GraphicsContext::BeginPass()
	{
		descHeaps->SetDescriptorHeaps(commandList.Get());
	}

	void GraphicsContext::EndPass()
	{
		FlushResourceBarriers();
		ResetStreamOutput();
	}

	void ComputeContext::SetRootSignature(RootSignatureRef rs)
	{
		commandList->SetComputeRootSignature(reinterpret_cast<ID3D12RootSignature *>(rs->GetImplDetail()));
	}

	void ComputeContext::SetPipelineState(PipelineStateRef pso)
	{
		commandList->SetPipelineState(reinterpret_cast<ID3D12PipelineState *>(pso->GetImplDetail()));
	}

	void ComputeContext::SetVertexBuffer(GpuResourceRef handle)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetIndexBuffer(GpuResourceRef handle)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::DrawIndexed(uint32_t indexCount)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::DrawIndexed(uint32_t indexCount, uint32_t vertexOffset)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::Draw(uint32_t vertexCount)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::Draw(uint32_t vertexCount, uint32_t vertexOffset)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
	{
		commandList->Dispatch(threadGroupX, threadGroupY, threadGroupZ);
	}

	void ComputeContext::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearUnorderedAccessViewUint(GpuResourceRef handle, const DirectX::XMUINT4 & values)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearRenderTarget(uint8_t idx)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearRenderTarget(uint8_t idx, const float * clearColor)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearDepthOnly()
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearStencilOnly()
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearDepthStencil()
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetStreamOutput(GpuResourceRef handle)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetStreamOutputFilledSize(GpuResourceRef handle, uint64_t byteOffset)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ResetStreamOutput()
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRenderTargets(std::initializer_list<GpuResourceRef> handles, GpuResourceRef depthStencil)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetStencilReference(uint8_t stencilValue)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetViewport(uint32_t width, uint32_t height)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetViewport()
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetScissorRect(uint32_t width, uint32_t height)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetScissorRect()
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRenderTargets(GpuResourceRef renderTarget, GpuResourceRef depthStencil)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRenderTargets(std::nullptr_t rt, std::nullptr_t ds)
	{

	}

	void ComputeContext::SetRenderTargets(std::nullptr_t rt, ResourceViewsRef ds)
	{
	}

	void ComputeContext::SetRenderTargets(ResourceViewsRef rt, std::nullptr_t ds)
	{
	}

	void ComputeContext::SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil)
	{
		//Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRenderTargets(std::nullptr_t rt, GpuResourceRef ds)
	{
	}

	void ComputeContext::SetRenderTargets(GpuResourceRef rt, std::nullptr_t ds)
	{
	}

	void ComputeContext::SetRootConstants(int slot, const void * srcData, uint32_t numConstants)
	{
		commandList->SetComputeRoot32BitConstants(slot, numConstants, srcData, 0);
	}

	void ComputeContext::SetShaderResources(int slot, std::initializer_list<GpuResourceRef> shaderResourceHandles) {
		D3D12_GPU_DESCRIPTOR_HANDLE descriptor;
		descriptor.ptr = 0;

		for(const GpuResourceRef & i : shaderResourceHandles) {
			DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(i);
			if(descriptor.ptr == 0) {
				descriptor = descHeaps->CreateSRV(rr);
			} else descHeaps->CreateSRV(rr);
		}

		commandList->SetComputeRootDescriptorTable(slot, descriptor);
	}

	void ComputeContext::SetShaderResources(int slot, ResourceViewsRef resourceView)
	{
		SetShaderResources(slot, resourceView, 0);
	}

	void ComputeContext::SetShaderResources(int slot, ResourceViewsRef resourceView, int descriptorOffset)
	{
		DX12ResourceViewsRef srv = std::dynamic_pointer_cast<DX12ResourceViews>(resourceView);

		commandList->SetComputeRootDescriptorTable(slot, srv->GetGpuHandle(descriptorOffset));
	}

	void ComputeContext::SetConstantBuffer(int slot, GpuResourceRef cbufferHandle)
	{
		DX12ResourceRef rr = std::dynamic_pointer_cast<DX12Resource>(cbufferHandle);

		commandList->SetComputeRootConstantBufferView(slot, rr->resource->GetGPUVirtualAddress());
	}

	void ComputeContext::SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)
	{
		uint64_t handle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);
		cbuffers->CopyData(handle, srcData, srcDataSizeInBytes);

		commandList->SetComputeRootConstantBufferView(slot, cbuffers->GetNativeHandle(handle));
	}

	void ComputeContext::CopyBufferRegion(GpuResourceRef dstResource, GpuResourceRef srcResource, size_t sizeInBytes)
	{
		CopyBufferRegion(dstResource, 0, srcResource, 0, sizeInBytes);
	}

	void ComputeContext::CopyBufferRegion(GpuResourceRef dstResource, size_t dstOffset, GpuResourceRef srcResource, size_t srcOffset, size_t sizeInBytes)
	{
		DX12ResourceRef rr0 = std::dynamic_pointer_cast<DX12Resource>(dstResource);
		DX12ResourceRef rr1 = std::dynamic_pointer_cast<DX12Resource>(srcResource);

		commandList->CopyBufferRegion(rr0->resource.Get(), dstOffset, rr1->resource.Get(), srcOffset, sizeInBytes);
	}

	void ComputeContext::BeginPass()
	{
		descHeaps->SetDescriptorHeaps(commandList.Get());
	}

	void ComputeContext::EndPass()
	{
		FlushResourceBarriers();
	}

}
