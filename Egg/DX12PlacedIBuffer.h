#pragma once

#include "DX12Resource.h"
#include "Utility.h"

namespace Egg::Graphics::DX12::Resource {

	class PlacedIBuffer : public AIBuffer {
		com_ptr<ID3D12Heap> resourceHeap;

		struct LODLevelResources {
			com_ptr<ID3D12Resource> resource;
			void * cpuResource;
			UINT sizeInBytes;
		};

		LODLevelResources lodResources[8];
	public:

		void AddLODLevel(ID3D12Device * device, void * vertexData, UINT sizeInBytes, DXGI_FORMAT format) {
			lodLevels[lodLevelsLength].indexBufferView.Format = format;
			lodLevels[lodLevelsLength].indexBufferView.SizeInBytes = sizeInBytes;
			lodLevels[lodLevelsLength].indexCount = sizeInBytes / 4U;
			lodResources[lodLevelsLength].cpuResource = vertexData;
			lodResources[lodLevelsLength].sizeInBytes = sizeInBytes;
			lodLevelsLength += 1;
		}

		virtual void UploadResources(IResourceUploader * uploader) override {
			for(UINT i = 0; i < lodLevelsLength; ++i) {
				uploader->Upload(lodResources[i].resource.Get(), reinterpret_cast<const BYTE*>(lodResources[i].cpuResource), lodResources[i].sizeInBytes);
				uploader->Transition(lodResources[i].resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			}
		}

		virtual void ReleaseUploadResources() override {

		}

		virtual void CreateResources(ID3D12Device * device) override {
			UINT heapSizeInBytes = 0;

			for(UINT i = 0; i < lodLevelsLength; ++i) {
				heapSizeInBytes += Egg::Utility::Align64K(lodLevels[i].indexBufferView.SizeInBytes);
			}

			D3D12_HEAP_DESC heapDesc;
			heapDesc.Alignment = 0x10000;
			heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
			heapDesc.SizeInBytes = heapSizeInBytes;
			heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

			DX_API("Failed to create heap")
				device->CreateHeap(&heapDesc, IID_PPV_ARGS(resourceHeap.GetAddressOf()));

			UINT offset = 0;
			for(UINT i = 0; i < lodLevelsLength; ++i) {
				DX_API("Failed to create placed resource")
					device->CreatePlacedResource(resourceHeap.Get(),
												 offset,
												 &CD3DX12_RESOURCE_DESC::Buffer(lodLevels[i].indexBufferView.SizeInBytes),
												 D3D12_RESOURCE_STATE_COPY_DEST,
												 nullptr,
												 IID_PPV_ARGS(lodResources[i].resource.GetAddressOf()));


				lodLevels[i].indexBufferView.BufferLocation = lodResources[i].resource->GetGPUVirtualAddress();
				offset += Utility::Align64K(lodLevels[i].indexBufferView.SizeInBytes);
			}

		}

		virtual void ReleaseResources() override {

		}



	};

}