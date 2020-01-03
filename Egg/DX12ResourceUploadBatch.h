#pragma once

#include <queue>
#include <DirectXTex/DirectXTex.h>
#include "DX12Resource.h"

namespace Egg::Graphics::DX12::Resource {

	class ResourceUploadBatch : public IResourceUploadBatch {

		struct CopyItem {
			ID3D12Resource * destResource;

			union {
				// buffer
				struct {
					const BYTE * buffer;
					UINT64 sizeInBytes;
				};
				struct {
					const DirectX::Image * images;
					UINT numImages;
				};
			};

			D3D12_RESOURCE_DIMENSION dimension;

			CopyItem(ID3D12Resource * resource, const BYTE * srcBuffer, UINT64 sizeInBytes) : 
				destResource{ resource }, buffer{ srcBuffer }, sizeInBytes{ sizeInBytes }, dimension{ D3D12_RESOURCE_DIMENSION_BUFFER } {

			}

			CopyItem(ID3D12Resource * resource, const DirectX::Image * images, UINT numImages = 1) :
				destResource{ resource }, images{ images }, numImages{ numImages }, dimension{ D3D12_RESOURCE_DIMENSION_TEXTURE2D } {

			}
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

		UINT64 CalcCopySize(const DirectX::Image* images, UINT numImages);

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

		void UpdateCopySize(UINT64 requiredSize);
	public:

		virtual void Upload(ID3D12Resource * destResource, const DirectX::Image * image) override;
		virtual void Upload(ID3D12Resource * destResource, const DirectX::Image * images, UINT numImages) override;
		virtual void Upload(ID3D12Resource * destResource, const BYTE * cpuResource, UINT64 sizeInBytes) override;

		//virtual void Upload(const D3D12_RESOURCE_DESC & resourceDesc, ID3D12Resource * destResource, void * cpuResource, UINT sizeInBytes) override;

		virtual void Transition(ID3D12Resource * resource, D3D12_RESOURCE_STATES preState, D3D12_RESOURCE_STATES postState) override;

		virtual void CreateResources(ID3D12Device * dev) override;

		virtual void ReleaseResources() override;

		virtual void Process(ID3D12CommandQueue * directQueue, ID3D12CommandQueue * copyQueue) override;
	};

}
