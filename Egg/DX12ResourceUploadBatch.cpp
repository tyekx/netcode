#include "DX12ResourceUploadBatch.h"
#include "Common.h"
#include "Utility.h"

namespace Egg::Graphics::DX12::Resource {

	UINT64 ResourceUploadBatch::CalcTex2DCopySize(const D3D12_RESOURCE_DESC & resourceDesc) {
		UINT64 copyableSize;
		device->GetCopyableFootprints(&resourceDesc, 0, resourceDesc.MipLevels, 0, nullptr, nullptr, nullptr, &copyableSize);
		return copyableSize;
	}

	UINT64 ResourceUploadBatch::CalcCopySize(const D3D12_RESOURCE_DESC & resourceDesc) {
		switch(resourceDesc.Dimension) {
		case D3D12_RESOURCE_DIMENSION_BUFFER:
			return resourceDesc.Width;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			return CalcTex2DCopySize(resourceDesc);
		default:
			ASSERT(false, "error");
			break;
		}
		return 0;
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
		UINT bytesPerPixel = static_cast<UINT>((DirectX::BitsPerPixel(item.resourceDesc.Format) / 8U));
		static D3D12_SUBRESOURCE_DATA subData[16];

		const bool isSupported = item.resourceDesc.MipLevels == 1 || (Egg::Utility::IsPowerOf2(static_cast<UINT>(item.resourceDesc.Width)) && Egg::Utility::IsPowerOf2(item.resourceDesc.Height));
		ASSERT(item.resourceDesc.Width < (1 << 16), "???");
		ASSERT(isSupported, "The current version only supports power of 2 dimensions for mip levels");

		UINT64 total = 0;
		UINT width = static_cast<UINT>(item.resourceDesc.Width);
		UINT height = item.resourceDesc.Height;
		for(UINT i = 0; i < item.resourceDesc.MipLevels; ++i) {
			subData[i].pData = reinterpret_cast<BYTE *>(item.cpuResource) + total;
			subData[i].RowPitch = width * bytesPerPixel;
			subData[i].SlicePitch = width * height * bytesPerPixel;
			total += subData[i].SlicePitch;
			width >>= 1;
			height >>= 1;
		}

		UpdateSubresources(copyCommandList.Get(), item.destResource, uploadResource.Get(), 0, 0, item.resourceDesc.MipLevels, subData);
	}

	void ResourceUploadBatch::CopyBuffer(CopyItem & item) {
		memcpy(mappedPtr, item.cpuResource, item.cpuResourceSizeInBytes);
		copyCommandList->CopyBufferRegion(item.destResource, 0, uploadResource.Get(), 0, item.resourceDesc.Width);
	}

	void ResourceUploadBatch::RecordCopyCommand(CopyItem & item) {
		// Texture2D
		switch(item.resourceDesc.Dimension)
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

	void ResourceUploadBatch::Upload(const D3D12_RESOURCE_DESC & resourceDesc, ID3D12Resource * destResource, void * cpuResource, UINT sizeInBytes) {
		items.push({ resourceDesc, destResource, cpuResource, sizeInBytes });
		UINT64 copySize = CalcCopySize(resourceDesc);

		if(copySize > maxCopySize) {
			maxCopySize = copySize;
		}
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
