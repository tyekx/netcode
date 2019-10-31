#pragma once

#include "Asset/Material.h"
#include "Resource.h"
#include <map>
#include <memory>

namespace Egg::Graphics::Internal {

	class TextureLibrary {
		ID3D12Device * device;
		std::map<std::string, std::unique_ptr<Resource::ITexture>> textures;
		com_ptr<ID3D12DescriptorHeap> descriptorHeap;
		UINT increment;
		UINT nextId;
		UINT maxDesc;

	public:
		TextureLibrary() : textures{}, descriptorHeap{}, increment{}, nextId{ 0 }, maxDesc{ 2048 } {}

		void LoadTexture(const std::string & name) {
			// @TODO: import and insert
		}

		void CreateResources(ID3D12Device * dev) {
			device = dev;
			D3D12_DESCRIPTOR_HEAP_DESC dhd;
			dhd.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			dhd.NodeMask = 0;
			dhd.NumDescriptors = maxDesc;
			dhd.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

			DX_API("Failed to create descriptor heap")
				device->CreateDescriptorHeap(&dhd, IID_PPV_ARGS(descriptorHeap.GetAddressOf()));

			increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		void HandleTexture(const std::string & name, D3D12_GPU_DESCRIPTOR_HANDLE & dst) {
			decltype(textures)::iterator it = textures.find(name);

			if(it == textures.end()) {
				LoadTexture(name);
				it = textures.find(name);
			}

			CD3DX12_CPU_DESCRIPTOR_HANDLE dh{ descriptorHeap->GetCPUDescriptorHandleForHeapStart() };
			dh.Offset(increment * nextId);
			it->second->CreateShaderResourceView(device, dh);

			CD3DX12_GPU_DESCRIPTOR_HANDLE gdh{ descriptorHeap->GetGPUDescriptorHandleForHeapStart() };
			gdh.Offset(increment * nextId);

			++nextId;

			if(dst.ptr == 0) {
				dst.ptr = gdh.ptr;
			}
		}

	public:
		D3D12_GPU_DESCRIPTOR_HANDLE LoadTextures(Asset::Material * mat) {
			if(!mat->HasTextures()) {
				return D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
			}

			D3D12_GPU_DESCRIPTOR_HANDLE firstCandidate{ 0 };

			if(mat->HasDiffuseTexture()) {
				HandleTexture(mat->diffuseTexture, firstCandidate);
			}

			if(mat->HasNormalTexture()) {
				HandleTexture(mat->normalTexture, firstCandidate);
			}

			return firstCandidate;
		}
	};

}
