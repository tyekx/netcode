#include "DX12ResourceUploadBatch.h"
#include "Common.h"
#include "Utility.h"

namespace Egg::Graphics::DX12::Resource {

	UINT64 ResourceUploadBatch::CalcCopySize(const DirectX::Image * images, UINT numImages) {
		UINT64 sum = 0;
		for(UINT i = 0; i < numImages; ++i) {
			sum += images[i].slicePitch;
		}
		return sum;
	}

	void ResourceUploadBatch::WaitForUpload() {
		const UINT64 fv = fenceValue;

		if(fence->GetCompletedValue() < fv) {
			DX_API("Failed to set event on completion")
				fence->SetEventOnCompletion(fv, fenceEvent);

			WaitForSingleObject(fenceEvent, INFINITE);
		}

		fenceValue += 1;
	}

	void ResourceUploadBatch::CopyTex2D(CopyItem & item) {
		static D3D12_SUBRESOURCE_DATA subData[16];

		ASSERT(item.numImages <= 16, "A maximum of 16 mipmap levels are supported");

		for(UINT i = 0; i < item.numImages; ++i) {
			subData[i].pData = item.images[i].pixels;
			subData[i].RowPitch = item.images[i].rowPitch;
			subData[i].SlicePitch = item.images[i].slicePitch;
		}

		_Analysis_assume_(item.numImages <= 16)
		UpdateSubresources(copyCommandList.Get(), item.destResource, uploadResource.Get(), 0, 0, item.numImages, subData);
	}

	void ResourceUploadBatch::CopyBuffer(CopyItem & item) {
		memcpy(mappedPtr, item.buffer, item.sizeInBytes);
		copyCommandList->CopyBufferRegion(item.destResource, 0, uploadResource.Get(), 0, item.sizeInBytes);
	}

	void ResourceUploadBatch::RecordCopyCommand(CopyItem & item) {
		switch(item.dimension)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			CopyTex2D(item);
			return;
		case D3D12_RESOURCE_DIMENSION_BUFFER:
			CopyBuffer(item);
			return;
		}

		ASSERT(false, "Not supported dimension");
	}

	void ResourceUploadBatch::PrepareUpload() {
		if(currentCopyBufferSize < maxCopySize) {
			if(uploadResource != nullptr) {
				// free previous resource to create a bigger one

				CD3DX12_RANGE writtenRange{ 0,0 };
				uploadResource->Unmap(0, &writtenRange);
				mappedPtr = nullptr;

				uploadResource.Reset();
			}
		} else return; // no need to allocate another buffer if the current one is big enough

		currentCopyBufferSize = maxCopySize;

		DX_API("Failed to create upload resource")
			device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
											D3D12_HEAP_FLAG_NONE,
											&CD3DX12_RESOURCE_DESC::Buffer(maxCopySize),
											D3D12_RESOURCE_STATE_GENERIC_READ,
											nullptr,
											IID_PPV_ARGS(uploadResource.GetAddressOf()));

		CD3DX12_RANGE readRange{ 0,0 };
		DX_API("Failed to map ptr")
			uploadResource->Map(0, &readRange, &mappedPtr);
	}

	void ResourceUploadBatch::ResetDirectCommandList() {
		DX_API("failed to reset command allocator")
			directCommandAlloc->Reset();

		DX_API("Failed to reset command list")
			directCommandList->Reset(directCommandAlloc.Get(), nullptr);
	}

	void ResourceUploadBatch::Prepare() {

	}

	void ResourceUploadBatch::ResetCopyCommandList() {
		DX_API("failed to reset command allocator")
			copyCommandAlloc->Reset();

		DX_API("Failed to reset command list")
			copyCommandList->Reset(copyCommandAlloc.Get(), nullptr);
	}

	void ResourceUploadBatch::CloseDirectCommandList() {
		DX_API("Failed to close to direct command list")
			directCommandList->Close();
	}

	void ResourceUploadBatch::CloseCopyCommandList() {
		DX_API("Failed to close copy command list")
			copyCommandList->Close();
	}

	bool ResourceUploadBatch::UploadOne(ID3D12CommandQueue * copyCommandQueue) {
		if(items.empty()) {
			return false;
		}

		ResetCopyCommandList();

		CopyItem & ci = items.front();

		RecordCopyCommand(ci);

		CloseCopyCommandList();

		ID3D12CommandList * cls[] = { copyCommandList.Get() };

		copyCommandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);

		copyCommandQueue->Signal(fence.Get(), fenceValue);

		items.pop();

		WaitForUpload();

		return true;
	}

	void ResourceUploadBatch::TransitionAll(ID3D12CommandQueue * directCommandQueue) {
		if(transitions.empty()) {
			return;
		}

		ResetDirectCommandList();

		while(!transitions.empty()) {
			const auto & transition = transitions.front();

			directCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(transition.resource, transition.preState, transition.postState));

			transitions.pop();
		}

		CloseDirectCommandList();

		ID3D12CommandList * cls[] = { directCommandList.Get() };

		directCommandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);

		directCommandQueue->Signal(fence.Get(), fenceValue);

		WaitForUpload();
	}

	void ResourceUploadBatch::UploadAll(ID3D12CommandQueue * copyCommandQueue) {
		while(UploadOne(copyCommandQueue));
	}

	void ResourceUploadBatch::UpdateCopySize(UINT64 requiredSize)
	{
		if(requiredSize > maxCopySize) {
			maxCopySize = requiredSize;
		}
	}

	void ResourceUploadBatch::Upload(ID3D12Resource * destResource, const DirectX::Image * image)
	{
		UpdateCopySize(CalcCopySize(image, 1));
		items.emplace(destResource, image);
	}

	void ResourceUploadBatch::Upload(ID3D12Resource * destResource, const DirectX::Image * images, UINT numImages)
	{
		UpdateCopySize(CalcCopySize(images, numImages));
		items.emplace(destResource, images, numImages);
	}

	void ResourceUploadBatch::Upload(ID3D12Resource * destResource, const BYTE * cpuResource, UINT64 sizeInBytes)
	{
		UpdateCopySize(sizeInBytes);

		items.emplace(destResource, cpuResource, sizeInBytes);
	}

	void ResourceUploadBatch::Transition(ID3D12Resource * resource, D3D12_RESOURCE_STATES preState, D3D12_RESOURCE_STATES postState) {
		transitions.push({ resource, preState, postState });
	}

	void ResourceUploadBatch::CreateResources(ID3D12Device * dev) {
		device = dev;

		DX_API("Failed to create copy command allocator")
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(copyCommandAlloc.GetAddressOf()));

		DX_API("Failed to create copy command list")
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, copyCommandAlloc.Get(), nullptr, IID_PPV_ARGS(copyCommandList.GetAddressOf()));

		DX_API("Failed to create direct command allocator")
			device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(directCommandAlloc.GetAddressOf()));

		DX_API("Failed to create direct command list")
			device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, directCommandAlloc.Get(), nullptr, IID_PPV_ARGS(directCommandList.GetAddressOf()));

		DX_API("Failed to initially close command list")
			copyCommandList->Close();

		DX_API("Failed to initially close command list")
			directCommandList->Close();

		DX_API("Failed to create fence")
			device->CreateFence(0, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(fence.GetAddressOf()));

		fenceValue = 1; currentCopyBufferSize = 0;

		fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		if(fenceEvent == NULL) {
			DX_API("Failed to create windows event") HRESULT_FROM_WIN32(GetLastError());
		}
	}

	void ResourceUploadBatch::ReleaseResources() {
		device = nullptr;
		uploadResource.Reset();
		fence.Reset();
		copyCommandAlloc.Reset();
		directCommandAlloc.Reset();
		copyCommandList.Reset();
		directCommandList.Reset();
		currentCopyBufferSize = 0;
	}

	void ResourceUploadBatch::Process(ID3D12CommandQueue * directQueue, ID3D12CommandQueue * copyQueue) {
		PrepareUpload();
		UploadAll(copyQueue);
		TransitionAll(directQueue);
		maxCopySize = 0;
	}

}
