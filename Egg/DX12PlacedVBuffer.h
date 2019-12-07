#pragma once

#include "DX12Resource.h"
#include "Utility.h"

namespace Egg::Graphics::DX12::Resource {

	class PlacedVBuffer : public AVBuffer {
		com_ptr<ID3D12Heap> resourceHeap;
		struct LODLevelResources {
			com_ptr<ID3D12Resource> resource;
			D3D12_RESOURCE_DESC resourceDesc;
			void * cpuResource;
			UINT sizeInBytes;
		};

		LODLevelResources lodResources[8];
	public:

		void AddLODLevel(ID3D12Device * device, void * vertexData, UINT sizeInBytes, UINT strideInBytes) {
			lodLevels[lodLevelsLength].vertexBufferView.StrideInBytes = strideInBytes;
			lodLevels[lodLevelsLength].vertexBufferView.SizeInBytes = sizeInBytes;
			lodLevels[lodLevelsLength].verticesCount = sizeInBytes / strideInBytes;
			lodResources[lodLevelsLength].cpuResource = vertexData;
			lodResources[lodLevelsLength].sizeInBytes = sizeInBytes;
			lodResources[lodLevelsLength].resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeInBytes);
			lodLevelsLength += 1;
		}

		virtual void UploadResources(IResourceUploader * uploader) override {
			for(UINT i = 0; i < lodLevelsLength; ++i) {
				uploader->Upload(lodResources[i].resourceDesc, lodResources[i].resource.Get(), lodResources[i].cpuResource, lodResources[i].sizeInBytes);
				uploader->Transition(lodResources[i].resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			}
		}

		virtual void ReleaseUploadResources() override {

		}

		virtual void CreateResources(ID3D12Device * device) override {
			UINT heapSizeInBytes = 0;

			for(UINT i = 0; i < lodLevelsLength; ++i) {
				heapSizeInBytes += Egg::Utility::Align64K(lodLevels[i].vertexBufferView.SizeInBytes);
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

				lodResources[i].resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(lodLevels[i].vertexBufferView.SizeInBytes);

				DX_API("Failed to create placed resource")
					device->CreatePlacedResource(resourceHeap.Get(),
												 offset,
											     &lodResources[i].resourceDesc,
												 D3D12_RESOURCE_STATE_COPY_DEST,
												 nullptr,
												 IID_PPV_ARGS(lodResources[i].resource.GetAddressOf()));


				lodLevels[i].vertexBufferView.BufferLocation = lodResources[i].resource->GetGPUVirtualAddress();
				offset += Utility::Align64K(lodLevels[i].vertexBufferView.SizeInBytes);
			}

		}

		virtual void ReleaseResources() override {

		}



	};

}
