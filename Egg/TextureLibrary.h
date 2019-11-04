#pragma once

#include "Asset/Material.h"
#include "Resource.h"
#include <map>
#include <memory>
#include "Importer.h"

namespace Egg::Graphics::Internal {

	class TextureLibrary {
		ID3D12Device * device;
		std::map<std::string, Resource::Committed::Texture2D> textures;
		com_ptr<ID3D12DescriptorHeap> descriptorHeap;
		UINT increment;
		UINT nextId;
		UINT maxDesc;

	public:
		TextureLibrary() : device{ nullptr }, textures {}, descriptorHeap{}, increment{}, nextId{ 0 }, maxDesc{ 2048 } {}

		void SetDescriptorHeap(ID3D12GraphicsCommandList * gcl) {
			ID3D12DescriptorHeap * dHeaps[] = { descriptorHeap.Get() };
			gcl->SetDescriptorHeaps(_countof(dHeaps), dHeaps);
		}

		void LoadTexture(const std::string & name) {
			// @TODO: import and insert

			auto tex = Importer::ImportCommittedTexture2D(device, Egg::Utility::ToWideString(name));
			
			textures[name] = tex;
		}

		void UploadResources(ID3D12GraphicsCommandList * gcl) {
			for(auto & i : textures) {
				i.second.UploadResources(gcl);
			}
		}

		void ReleaseUploadResources() {
			for(auto & i : textures) {
				i.second.ReleaseUploadResources();
			}
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

			D3D12_CPU_DESCRIPTOR_HANDLE cpuDh = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
			cpuDh.ptr += increment * nextId;
			it->second.CreateShaderResourceView(device, cpuDh);

			D3D12_GPU_DESCRIPTOR_HANDLE gpuDh{ descriptorHeap->GetGPUDescriptorHandleForHeapStart() };
			gpuDh.ptr += increment * nextId;

			++nextId;

			if(dst.ptr == 0) {
				dst.ptr = gpuDh.ptr;
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
