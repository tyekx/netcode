#include "CommittedIBuffer.h"

namespace Egg::Graphics::Resource::Committed {

	void IBuffer::SetDesc(const D3D12_RESOURCE_DESC & resDesc) {
		resourceDesc = resDesc;
	}

	void IBuffer::CopyToUploadBuffer(const void * srcData, UINT64 sizeInBytes) {
		CD3DX12_RANGE readRange{ 0,0 };
		void * mappedPtr;

		DX_API("Failed to map upload resource")
			uploadResource->Map(0, &readRange, &mappedPtr);

		memcpy(mappedPtr, srcData, sizeInBytes);

		uploadResource->Unmap(0, nullptr);
	}

	void IBuffer::SetFormat(DXGI_FORMAT format) {
		indexBufferView.Format = format;
	}

	void IBuffer::CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & resDesc, const void * srcData, UINT64 sizeInBytes, DXGI_FORMAT format) {
		SetDesc(resDesc);
		SetFormat(format);
		CreateResources(device);
		CopyToUploadBuffer(srcData, sizeInBytes);
	}

	void IBuffer::CreateResources(ID3D12Device * device) {
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

		indexBufferView.BufferLocation = resource->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = resourceDesc.Width;
	}

	void IBuffer::ReleaseResources() {
		resource.Reset();
	}

	void IBuffer::UploadResources(ID3D12GraphicsCommandList * copyCommandList) {
		if(uploadResource == nullptr) {
			return;
		}
		copyCommandList->CopyBufferRegion(resource.Get(), 0, uploadResource.Get(), 0, resourceDesc.Width); 
		copyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER));
	}

	void IBuffer::ReleaseUploadResources() {
		uploadResource.Reset();
	}

}
