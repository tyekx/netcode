#include "DX12CommittedIBuffer.h"
// @TODO revise legacy code
/*
namespace Egg::Graphics::DX12::Resource {

	void CommittedIBuffer::SetDesc(const D3D12_RESOURCE_DESC & resDesc) {
		resourceDesc = resDesc;
	}

	void CommittedIBuffer::CopyToUploadBuffer(const void * srcData, UINT64 sizeInBytes) {
		CD3DX12_RANGE readRange{ 0,0 };
		void * mappedPtr;

		DX_API("Failed to map upload resource")
			uploadResource->Map(0, &readRange, &mappedPtr);

		memcpy(mappedPtr, srcData, sizeInBytes);

		uploadResource->Unmap(0, nullptr);
	}

	void CommittedIBuffer::SetFormat(DXGI_FORMAT format) {
		//indexBufferView.Format = format;
	}

	void CommittedIBuffer::CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & resDesc, const void * srcData, UINT64 sizeInBytes, DXGI_FORMAT format) {
		SetDesc(resDesc);
		SetFormat(format);
		CreateResources(device);
		CopyToUploadBuffer(srcData, sizeInBytes);
	}

	void CommittedIBuffer::CreateResources(ID3D12Device * device) {
		DX_API("Failed to upload buffer for committed vertex buffer")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(uploadResource.GetAddressOf()));

		DX_API("Failed to create dimension buffer in default heap for committed vertex buffer")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(resource.GetAddressOf()));

		//indexBufferView.BufferLocation = resource->GetGPUVirtualAddress();
		//indexBufferView.SizeInBytes = resourceDesc.Width;
	}

	void CommittedIBuffer::ReleaseResources() {
		resource.Reset();
	}

	void CommittedIBuffer::UploadResources(IResourceUploader * uploader) {/*
		if(uploadResource == nullptr) {
			return;
		}
		copyCommandList->CopyBufferRegion(resource.Get(), 0, uploadResource.Get(), 0, resourceDesc.Width); 
		copyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	}

	void CommittedIBuffer::ReleaseUploadResources() {
		uploadResource.Reset();
	}

}
*/