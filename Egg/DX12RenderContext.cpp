#include "DX12RenderContext.h"
#include "DX12SpriteFont.h"

namespace Egg::Graphics::DX12 {

	BaseRenderContext::BaseRenderContext(ResourcePool * resourcePool, ConstantBufferPool * cbufferPool, DynamicDescriptorHeap * dHeaps, com_ptr<ID3D12GraphicsCommandList> cl) :
		resources{ resourcePool }, cbuffers{ cbufferPool }, descHeaps{ dHeaps }, commandList{ std::move(cl) }, signalFence{}, waitFences{}, barriers{}
	{

	}

	void BaseRenderContext::ResourceBarrier(uint64_t handle, ResourceState before, ResourceState after)
	{
		ID3D12Resource * res = resources->GetNativeResource(handle).resource;

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

	void GraphicsContext::SetVertexBuffer(uint64_t handle)
	{
		const GResource & res = resources->GetNativeResource(handle);

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = res.address;
		vbv.SizeInBytes = static_cast<UINT>(res.desc.sizeInBytes);
		vbv.StrideInBytes = res.desc.strideInBytes;

		commandList->IASetVertexBuffers(0, 1, &vbv);
	}

	void GraphicsContext::SetIndexBuffer(uint64_t handle)
	{
		const GResource & res = resources->GetNativeResource(handle);
		
		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = res.address;
		ibv.Format = res.desc.format;
		ibv.SizeInBytes = static_cast<UINT>(res.desc.sizeInBytes);

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
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void GraphicsContext::Signal(FenceRef fence)
	{
		if(signalFence != nullptr) {
			Log::Warn("ComputeContext: signalFence is already set, overwriting");
		}
		signalFence = std::move(fence);
	}

	void GraphicsContext::Wait(FenceRef fence)
	{
		waitFences.emplace_back(std::move(fence));
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

	void GraphicsContext::ClearUnorderedAccessViewUint(uint64_t handle, const DirectX::XMUINT4 & values)
	{
		const GResource & res = resources->GetNativeResource(handle);

		const auto [cpuDesc, gpuDesc] = descHeaps->CreateBufferUAV(res);

		commandList->ClearUnorderedAccessViewUint(gpuDesc, cpuDesc, res.resource, &values.x, 0, nullptr);
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

	void GraphicsContext::SetStreamOutput(uint64_t handle)
	{
		const GResource & res = resources->GetNativeResource(handle);

		D3D12_STREAM_OUTPUT_BUFFER_VIEW sobv;
		sobv.BufferLocation = res.address;
		sobv.BufferFilledSizeLocation = streamOutput_FilledSizeLocation;
		sobv.SizeInBytes = res.desc.sizeInBytes;
		commandList->SOSetTargets(0, 1, &sobv);
	}

	void GraphicsContext::SetStreamOutputFilledSize(uint64_t handle, uint64_t byteOffset)
	{
		const GResource & res = resources->GetNativeResource(handle);

		streamOutput_FilledSizeLocation = res.address + ((byteOffset + 3ull) & ~(3ull));
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

	void GraphicsContext::SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil)
	{
		uint8_t k = 0;
		for(uint64_t rtHandle : handles) {
			ASSERT(rtHandle != 0, "SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil): handles array must not contain undefined (=0) handle");
			const GResource & res = resources->GetNativeResource(rtHandle);

			currentlyBoundRenderTargets[k++] = descHeaps->CreateRTV(res);
		}

		if(depthStencil == 0) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			const GResource & res = resources->GetNativeResource(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(res);
		}

		commandList->OMSetRenderTargets(static_cast<uint32_t>(handles.size()), currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetRenderTargets(uint64_t handle, uint64_t depthStencil)
	{
		if(handle == 0) {
			currentlyBoundRenderTargets[0] = backbuffer;
		} else {
			const GResource & res = resources->GetNativeResource(handle);
			currentlyBoundRenderTargets[0] = descHeaps->CreateRTV(res);
		}

		if(depthStencil == 0) {
			currentlyBoundDepth = backbufferDepth;
		} else {
			const GResource & res = resources->GetNativeResource(depthStencil);
			currentlyBoundDepth = descHeaps->CreateDSV(res);
		}

		commandList->OMSetRenderTargets(1, currentlyBoundRenderTargets, FALSE, &currentlyBoundDepth);
	}

	void GraphicsContext::SetShaderResources(int slot, std::initializer_list<uint64_t> shaderResourceHandles) {
		

		D3D12_GPU_DESCRIPTOR_HANDLE descriptor;
		descriptor.ptr = 0;
		
		for(uint64_t i : shaderResourceHandles) {
			const GResource & gres = resources->GetNativeResource(i);
			if(descriptor.ptr == 0) {
				descriptor = descHeaps->CreateSRV(gres);
			} else descHeaps->CreateSRV(gres);
		}

		commandList->SetGraphicsRootDescriptorTable(slot, descriptor);
	}

	void GraphicsContext::SetShaderResources(int slot, ResourceViewsRef resourceView)
	{
		DX12ResourceViewsRef srv = std::dynamic_pointer_cast<DX12ResourceViews>(resourceView);
		
		commandList->SetGraphicsRootDescriptorTable(slot, srv->GetGpuHandle(0));
	}
	
	void GraphicsContext::SetRootConstants(int slot, const void * srcData, uint32_t numConstants) {
		commandList->SetGraphicsRoot32BitConstants(slot, numConstants, srcData, 0);
	}

	void GraphicsContext::SetConstantBuffer(int slot, uint64_t cbufferHandle)
	{
		Log::Warn("SetConstantBuffer(int slot, uint64_t cbufferHandle) not implemneted");
	}

	void GraphicsContext::SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)
	{
		uint64_t handle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);
		cbuffers->CopyData(handle, srcData, srcDataSizeInBytes);

		commandList->SetGraphicsRootConstantBufferView(slot, cbuffers->GetNativeHandle(handle));
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

	void ComputeContext::SetVertexBuffer(uint64_t handle)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetIndexBuffer(uint64_t handle)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::DrawIndexed(uint32_t indexCount)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::DrawIndexed(uint32_t indexCount, uint32_t vertexOffset)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::Draw(uint32_t vertexCount)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::Draw(uint32_t vertexCount, uint32_t vertexOffset)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ)
	{
	}

	void ComputeContext::Signal(FenceRef fence)
	{
		if(signalFence != nullptr) {
			Log::Warn("ComputeContext: signalFence is already set, overwriting");
		}
		signalFence = std::move(fence);
	}

	void ComputeContext::Wait(FenceRef fence)
	{
		waitFences.emplace_back(std::move(fence));
	}

	void ComputeContext::SetPrimitiveTopology(PrimitiveTopology topology)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearUnorderedAccessViewUint(uint64_t handle, const DirectX::XMUINT4 & values)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearRenderTarget(uint8_t idx)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearRenderTarget(uint8_t idx, const float * clearColor)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearDepthOnly()
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearStencilOnly()
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ClearDepthStencil()
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetStreamOutput(uint64_t handle)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetStreamOutputFilledSize(uint64_t handle, uint64_t byteOffset)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::ResetStreamOutput()
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRenderTargets(std::initializer_list<uint64_t> handles, uint64_t depthStencil)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetStencilReference(uint8_t stencilValue)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetViewport(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetViewport(uint32_t width, uint32_t height)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetViewport()
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetScissorRect(uint32_t left, uint32_t right, uint32_t top, uint32_t bottom)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetScissorRect(uint32_t width, uint32_t height)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetScissorRect()
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRenderTargets(uint64_t renderTarget, uint64_t depthStencil)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRenderTargets(ResourceViewsRef renderTargets, ResourceViewsRef depthStencil)
	{
		Log::Debug("call to " __FUNCTION__ " is ignored");
	}

	void ComputeContext::SetRootConstants(int slot, const void * srcData, uint32_t numConstants)
	{
		commandList->SetComputeRoot32BitConstants(slot, numConstants, srcData, 0);
	}

	void ComputeContext::SetShaderResources(int slot, std::initializer_list<uint64_t> shaderResourceHandles) {


		D3D12_GPU_DESCRIPTOR_HANDLE descriptor;
		descriptor.ptr = 0;

		for(uint64_t i : shaderResourceHandles) {
			const GResource & gres = resources->GetNativeResource(i);
			if(descriptor.ptr == 0) {
				descriptor = descHeaps->CreateSRV(gres);
			} else descHeaps->CreateSRV(gres);
		}

		commandList->SetComputeRootDescriptorTable(slot, descriptor);
	}

	void ComputeContext::SetShaderResources(int slot, ResourceViewsRef resourceView)
	{
		DX12ResourceViewsRef srv = std::dynamic_pointer_cast<DX12ResourceViews>(resourceView);

		commandList->SetComputeRootDescriptorTable(slot, srv->GetGpuHandle(0));
	}

	void ComputeContext::SetConstantBuffer(int slot, uint64_t cbufferHandle)
	{
		Log::Warn("SetConstantBuffer(int slot, uint64_t cbufferHandle) not implemneted");
	}

	void ComputeContext::SetConstants(int slot, const void * srcData, size_t srcDataSizeInBytes)
	{
		uint64_t handle = cbuffers->CreateConstantBuffer(srcDataSizeInBytes);
		cbuffers->CopyData(handle, srcData, srcDataSizeInBytes);

		commandList->SetComputeRootConstantBufferView(slot, cbuffers->GetNativeHandle(handle));
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
