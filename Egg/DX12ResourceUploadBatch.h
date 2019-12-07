#pragma once

#include "Common.h"
#include <queue>
#include <DirectXTex/DirectXTex.h>
#include "DX12Resource.h"

namespace Egg::Graphics::DX12::Resource {

	class ResourceUploadBatch : public IResourceUploadBatch {

		struct CopyItem {
			D3D12_RESOURCE_DESC resourceDesc;
			ID3D12Resource * destResource;
			void * cpuResource;
			UINT cpuResourceSizeInBytes;
		};

		ID3D12Device * device;
		com_ptr<ID3D12Resource> uploadResource;
		com_ptr<ID3D12Fence> fence;
		com_ptr<ID3D12CommandAllocator> copyCommandAlloc;
		com_ptr<ID3D12CommandAllocator> directCommandAlloc;
		com_ptr<ID3D12GraphicsCommandList2> copyCommandList;
		com_ptr<ID3D12GraphicsCommandList2> directCommandList;
		UINT maxCopySize;
		UINT currentCopyBufferSize;

		UINT64 fenceValue;
		HANDLE fenceEvent;

		void * mappedPtr;

		std::queue<CopyItem> items;
		bool hasTransitions;

		UINT CalcTex2DCopySize(const D3D12_RESOURCE_DESC & resourceDesc) {
			UINT64 copyableSize;
			device->GetCopyableFootprints(&resourceDesc, 0, resourceDesc.MipLevels, 0, nullptr, nullptr, nullptr, &copyableSize);
			return static_cast<UINT>(copyableSize);
		}

		UINT CalcCopySize(const D3D12_RESOURCE_DESC & resourceDesc) {
			switch(resourceDesc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_BUFFER:
				return static_cast<UINT>(resourceDesc.Width);
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				return CalcTex2DCopySize(resourceDesc);
			default:
				ASSERT(false, "error");
				break;
			}
			return 0;
		}

		void WaitForUpload() {
			const UINT64 fv = fenceValue;

			if(fence->GetCompletedValue() < fv) {
				DX_API("Failed to set event on completion")
					fence->SetEventOnCompletion(fv, fenceEvent);

				WaitForSingleObject(fenceEvent, INFINITE);
			}
			
			fenceValue += 1;
		}


		void CopyTex2D(CopyItem & item) {
			UINT offset = 0;
			UINT bytesPerPixel = static_cast<UINT>((DirectX::BitsPerPixel(item.resourceDesc.Format) / 8U));
			for(UINT i = 0; i < item.resourceDesc.MipLevels; ++i) {
				CD3DX12_TEXTURE_COPY_LOCATION dst{ item.destResource, i };

				UINT width = ((UINT)item.resourceDesc.Width) >> i;
				UINT height = ((UINT)item.resourceDesc.Height) >> i;

				D3D12_PLACED_SUBRESOURCE_FOOTPRINT psf;
				psf.Offset = offset;
				psf.Footprint.Depth = 1;
				psf.Footprint.Height = height;
				psf.Footprint.Width = width;
				psf.Footprint.RowPitch = psf.Footprint.Width * bytesPerPixel;
				psf.Footprint.Format = item.resourceDesc.Format;
				offset += height * width * bytesPerPixel;
				CD3DX12_TEXTURE_COPY_LOCATION src{ uploadResource.Get(), psf };
				copyCommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
			}
		}

		void CopyBuffer(CopyItem & item) {
			copyCommandList->CopyBufferRegion(item.destResource, 0, uploadResource.Get(), 0, item.resourceDesc.Width);
		}

		void RecordCopyCommand(CopyItem & item) {
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

		void PrepareUpload() {
			if(currentCopyBufferSize < maxCopySize) {
				if(uploadResource != nullptr) {
					// free previous resource to create a bigger one

					CD3DX12_RANGE writtenRange{ 0,0 };
					uploadResource->Unmap(0, &writtenRange);
					mappedPtr = nullptr;

					uploadResource.Reset();
				}
			} else return; // no need to allocate another buffer if the current one is big enough


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

		void ResetDirectCommandList() {
			DX_API("failed to reset command allocator")
				directCommandAlloc->Reset();

			DX_API("Failed to reset command list")
				directCommandList->Reset(directCommandAlloc.Get(), nullptr);
		}

		virtual void Prepare() override {
			ResetDirectCommandList();
		}

		void ResetCopyCommandList() {
			DX_API("failed to reset command allocator")
				copyCommandAlloc->Reset();

			DX_API("Failed to reset command list")
				copyCommandList->Reset(copyCommandAlloc.Get(), nullptr);
		}

		void CloseDirectCommandList() {
			DX_API("Failed to close to direct command list")
				directCommandList->Close();
		}

		void CloseCopyCommandList() {
			DX_API("Failed to close copy command list")
				copyCommandList->Close();
		}


		bool UploadOne(ID3D12CommandQueue * copyCommandQueue) {
			if(items.empty()) {
				return false;
			}

			ResetCopyCommandList();

			CopyItem & ci = items.front();

			memcpy(mappedPtr, ci.cpuResource, ci.cpuResourceSizeInBytes);

			RecordCopyCommand(ci);

			CloseCopyCommandList();

			ID3D12CommandList * cls[] = { copyCommandList.Get() };

			copyCommandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);

			copyCommandQueue->Signal(fence.Get(), fenceValue);

			items.pop();

			WaitForUpload();

			return true;
		}

		void TransitionAll(ID3D12CommandQueue * directCommandQueue) {
			if(!hasTransitions) {
				return;
			}

			CloseDirectCommandList();

			ID3D12CommandList * cls[] = { directCommandList.Get() };

			directCommandQueue->ExecuteCommandLists(ARRAYSIZE(cls), cls);

			directCommandQueue->Signal(fence.Get(), fenceValue);

			hasTransitions = false;

			WaitForUpload();
		}

		void UploadAll(ID3D12CommandQueue * copyCommandQueue) {
			while(UploadOne(copyCommandQueue));
		}
	public:
		virtual void Upload(const D3D12_RESOURCE_DESC & resourceDesc, ID3D12Resource * destResource, void * cpuResource, UINT sizeInBytes) override {
			items.push({ resourceDesc, destResource, cpuResource, sizeInBytes });
			UINT copySize = CalcCopySize(resourceDesc);

			if(copySize > maxCopySize) {
				maxCopySize = copySize;
			}
		}

		virtual void Transition(ID3D12Resource * resource, D3D12_RESOURCE_STATES preState, D3D12_RESOURCE_STATES postState) override {
			directCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, preState, postState));
			hasTransitions = true;
		}

		virtual void CreateResources(ID3D12Device * dev) override {
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

		virtual void ReleaseResources() override {
			device = nullptr;
			uploadResource.Reset();
			fence.Reset();
			copyCommandAlloc.Reset();
			directCommandAlloc.Reset();
			copyCommandList.Reset();
			directCommandList.Reset();
			currentCopyBufferSize = 0;
		}

		virtual void Process(ID3D12CommandQueue * directQueue, ID3D12CommandQueue * copyQueue) override {
			PrepareUpload();
			UploadAll(copyQueue);
			TransitionAll(directQueue);
			maxCopySize = 0;
		}
	};

}
