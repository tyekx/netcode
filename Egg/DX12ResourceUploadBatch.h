#pragma once

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

		struct TransitionItem {
			ID3D12Resource * resource;
			D3D12_RESOURCE_STATES preState;
			D3D12_RESOURCE_STATES postState;
		};

		ID3D12Device * device;
		com_ptr<ID3D12Resource> uploadResource;
		com_ptr<ID3D12Fence> fence;
		com_ptr<ID3D12CommandAllocator> copyCommandAlloc;
		com_ptr<ID3D12CommandAllocator> directCommandAlloc;
		com_ptr<ID3D12GraphicsCommandList2> copyCommandList;
		com_ptr<ID3D12GraphicsCommandList2> directCommandList;
		UINT64 maxCopySize;
		UINT64 currentCopyBufferSize;

		UINT64 fenceValue;
		HANDLE fenceEvent;

		void * mappedPtr;

		std::queue<CopyItem> items;
		std::queue<TransitionItem> transitions;

		UINT64 CalcTex2DCopySize(const D3D12_RESOURCE_DESC & resourceDesc);

		UINT64 CalcCopySize(const D3D12_RESOURCE_DESC & resourceDesc);

		void WaitForUpload();


		void CopyTex2D(CopyItem & item);

		void CopyBuffer(CopyItem & item);

		void RecordCopyCommand(CopyItem & item);

		void PrepareUpload();

		void ResetDirectCommandList();

		virtual void Prepare() override;

		void ResetCopyCommandList();

		void CloseDirectCommandList();

		void CloseCopyCommandList();


		bool UploadOne(ID3D12CommandQueue * copyCommandQueue);

		void TransitionAll(ID3D12CommandQueue * directCommandQueue);

		void UploadAll(ID3D12CommandQueue * copyCommandQueue);
	public:
		virtual void Upload(const D3D12_RESOURCE_DESC & resourceDesc, ID3D12Resource * destResource, void * cpuResource, UINT sizeInBytes) override;

		virtual void Transition(ID3D12Resource * resource, D3D12_RESOURCE_STATES preState, D3D12_RESOURCE_STATES postState) override;

		virtual void CreateResources(ID3D12Device * dev) override;

		virtual void ReleaseResources() override;

		virtual void Process(ID3D12CommandQueue * directQueue, ID3D12CommandQueue * copyQueue) override;
	};

}
