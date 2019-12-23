#include "DX12CommittedVBuffer.h"

namespace Egg::Graphics::DX12::Resource {

	void CommittedVBuffer::SetStride(UINT strideInBytes) {
		stride = strideInBytes;
	}

	void CommittedVBuffer::AddLODLevel(ID3D12Device * device, const void * srcData, UINT64 sizeInBytes, UINT strideInBytes) {
		SetStride(strideInBytes);

		LODLevelResources lodRes;
		lodRes.sizeInBytes = sizeInBytes;
		lodRes.srcData = srcData;
		DX_API("Failed to create dimension buffer in default heap for committed vertex buffer")
			device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
				D3D12_HEAP_FLAG_NONE,
				&CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes),
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(lodRes.resource.GetAddressOf()));

		lodLevels.push_back(std::move(lodRes));
	}

	void CommittedVBuffer::CreateResources(ID3D12Device * device) {

	}

	void CommittedVBuffer::ReleaseResources() {
		lodLevels.clear();
	}

	void CommittedVBuffer::UploadResources(IResourceUploader * uploader) {
		for(auto & i : lodLevels) {
			uploader->Upload(i.resource.Get(), reinterpret_cast<const BYTE*>(i.srcData), i.sizeInBytes);
			uploader->Transition(i.resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		}
	}

	void CommittedVBuffer::ReleaseUploadResources() {

	}

}
