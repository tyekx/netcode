#pragma once

#include "DX12Common.h"
#include "HandleTypes.h"
#include "Importer.h"

#include "DX12Resource.h"
#include "DX12RenderItem.h"

namespace Egg::Graphics::DX12 {

	class TextureLibrary {

		struct Item {
			HTEXTURE handle;
			std::wstring path;
			std::unique_ptr<Resource::ITexture> texture;
		};

		std::vector<Item> items;
		std::vector<Resource::IUploadResource *> uploadPile;

		com_ptr<ID3D12DescriptorHeap> texturesHeap;
		UINT texturesNumDescriptors;
		UINT nextDescriptor;
		UINT incrementSize;

		HTEXTURE nextTextureId;

		HTEXTURE Get(const std::wstring & path) const {
			for(const Item & i : items) {
				if(i.path == path) {
					return i.handle;
				}
			}
			return 0xFFFFFFFF;
		}

		Resource::ITexture * Get(HTEXTURE handle) const {
			for(const Item & i : items) {
				if(i.handle == handle) {
					return i.texture.get();
				}
			}
			return nullptr;
		}

		ID3D12Device * device;

	public:

		void SetDescriptorHeap(ID3D12GraphicsCommandList * gcl) {
			ID3D12DescriptorHeap * dheaps[] = { texturesHeap.Get() };
			gcl->SetDescriptorHeaps(ARRAYSIZE(dheaps), dheaps);
		}

		void CreateResources(ID3D12Device * dev) {
			device = dev;

			texturesNumDescriptors = 2048;

			D3D12_DESCRIPTOR_HEAP_DESC dhd;
			dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			dhd.NodeMask = 0;
			dhd.NumDescriptors = texturesNumDescriptors;
			dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

			DX_API("Failed to create descriptor heap")
				device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(texturesHeap.GetAddressOf()));

			incrementSize = device->GetDescriptorHandleIncrementSize(dhd.Type);
			nextDescriptor = 0;
		}

		void AllocateTextures(D3D12_GPU_DESCRIPTOR_HANDLE & startHandle, D3D12_CPU_DESCRIPTOR_HANDLE & startCpuHandle, UINT numTextures) {
			if(numTextures == 0) {
				return;
			}

			ASSERT((nextDescriptor + numTextures) < texturesNumDescriptors, "Descriptor overflow");

			D3D12_GPU_DESCRIPTOR_HANDLE gdh = texturesHeap->GetGPUDescriptorHandleForHeapStart();
			gdh.ptr = gdh.ptr + nextDescriptor * incrementSize;

			D3D12_CPU_DESCRIPTOR_HANDLE cdh = texturesHeap->GetCPUDescriptorHandleForHeapStart();
			cdh.ptr = cdh.ptr + nextDescriptor * incrementSize;

			startHandle = gdh;
			startCpuHandle = cdh;

			nextDescriptor += numTextures;
		}

		void AllocateTextures(RenderItem* renderItem, UINT numTextures) {
			if(numTextures == 0) {
				return;
			}

			ASSERT((nextDescriptor + numTextures) < texturesNumDescriptors, "Descriptor overflow");

			D3D12_GPU_DESCRIPTOR_HANDLE gdh = texturesHeap->GetGPUDescriptorHandleForHeapStart();
			gdh.ptr = gdh.ptr + nextDescriptor * incrementSize;

			renderItem->texturesId = nextDescriptor;
			renderItem->texturesLength = numTextures;
			renderItem->texturesHandle = gdh;

			nextDescriptor += numTextures;
		}

		void UploadResources(Resource::IResourceUploader * uploader) {
			for(Resource::IUploadResource * r : uploadPile) {
				r->UploadResources(uploader);
			}
			uploadPile.clear();
		}

		void SetTexture(RenderItem * renderItem, UINT localIdx, HTEXTURE texture) {
			ASSERT(renderItem->texturesLength > localIdx, "Setting texture is out of bounds");
			
			const UINT absoluteIdx = renderItem->texturesId + localIdx;
			const D3D12_CPU_DESCRIPTOR_HANDLE cdh = CD3DX12_CPU_DESCRIPTOR_HANDLE{ texturesHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(absoluteIdx), incrementSize };

			const auto * itexture = Get(texture);
			if(itexture == nullptr) {
				return;
			}
			const D3D12_SHADER_RESOURCE_VIEW_DESC srvd = itexture->GetSRV();

			device->CreateShaderResourceView(itexture->GetResource(), &srvd, cdh);
		}



		HTEXTURE LoadTexture2D(const std::wstring & textureMediaPath) {
			HTEXTURE handle = Get(textureMediaPath);

			if(handle != 0xFFFFFFFF) {
				return handle;
			}

			std::unique_ptr<Resource::ITexture> texture = Importer::ImportCommittedTexture2D(device, textureMediaPath);

			uploadPile.push_back(texture.get());

			handle = nextTextureId++;

			Item item;
			item.handle = handle;
			item.path = textureMediaPath;
			item.texture = std::move(texture);

			items.emplace_back(std::move(item));

			return handle;
		}
		
	};

}

