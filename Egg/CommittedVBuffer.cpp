#include "CommittedVBuffer.h"

namespace Egg::Graphics::Resource::Committed {

	void VBuffer::SetDesc(const D3D12_RESOURCE_DESC & resDesc) {
		resourceDesc = resDesc;
	}

	void VBuffer::CopyToUploadBuffer(const void * srcData, UINT64 sizeInBytes) {
		CD3DX12_RANGE readRange{ 0,0 };
		void * mappedPtr;

		DX_API("Failed to map upload resource")
			uploadResource->Map(0, &readRange, &mappedPtr);

		memcpy(mappedPtr, srcData, sizeInBytes);

		uploadResource->Unmap(0, nullptr);
	}

	void VBuffer::SetStride(UINT strideInBytes) {
		vertexBufferView.StrideInBytes = strideInBytes;
	}

	void VBuffer::CreateResources(ID3D12Device * device, const D3D12_RESOURCE_DESC & desc, const void * srcData, UINT64 sizeInBytes, UINT strideInBytes) {
		SetDesc(desc);
		SetStride(strideInBytes);
		CreateResources(device);
		CopyToUploadBuffer(srcData, sizeInBytes);
	}

	void VBuffer::CreateResources(ID3D12Device * device) {
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

		vertexBufferView.BufferLocation = resource->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = resourceDesc.Width;
	}

	void VBuffer::ReleaseResources() {
		resource.Reset();
	}

	void VBuffer::UploadResources(ID3D12GraphicsCommandList * copyCommandList) {
		if(uploadResource == nullptr) {
			return;
		}
		copyCommandList->CopyBufferRegion(resource.Get(), 0, uploadResource.Get(), 0, resourceDesc.Width);
		copyCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	}

	void VBuffer::ReleaseUploadResources() {
		uploadResource.Reset();
	}

}
