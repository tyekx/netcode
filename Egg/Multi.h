#pragma once

#include "Material.h"
#include "Mesh.h"

namespace Egg {

	struct Multi {
		UINT length;
		std::unique_ptr<Material[]> material;
		std::unique_ptr<Mesh[]> meshes;
		PerObjectCb * perObjectCb;
		BoneDataCb * boneDataCb;
		D3D12_GPU_VIRTUAL_ADDRESS perObjectCbAddr;

		void InitWithLength(UINT len) {
			length = len;
			material = std::make_unique<Material[]>(length);
			meshes = std::make_unique<Mesh[]>(length);
		}

		inline void Draw(ID3D12GraphicsCommandList * gcl, D3D12_GPU_VIRTUAL_ADDRESS perFrameData) {
			for(UINT i = 0; i < length; ++i) {
				material[i].SetPipelineState(gcl);
				material[i].BindConstantBuffer(gcl, PerObjectCb::id, perObjectCbAddr);
				material[i].BindConstantBuffer(gcl, PerFrameCb::id, perFrameData);
				meshes[i].Draw(gcl);
			}
		}

		Multi(UINT len) : Multi{} {
			InitWithLength(len);
		}

		Multi() : length{ 0 }, material{ nullptr }, meshes{ nullptr }, perObjectCb{ nullptr }, boneDataCb{ nullptr }, perObjectCbAddr{ 0 } { }
		~Multi() = default;

		Multi(const Multi & m) : Multi{} {
			InitWithLength(m.length);

			boneDataCb = m.boneDataCb;
			perObjectCb = m.perObjectCb;
			perObjectCbAddr = m.perObjectCbAddr;
			for(UINT i = 0; i < length; ++i) {
				meshes[i] = m.meshes[i];
				material[i] = m.material[i];
			}
		}

		Multi(Multi && m) noexcept : Multi{} {
			std::swap(length, m.length);
			std::swap(material, m.material);
			std::swap(meshes, m.meshes);
			std::swap(boneDataCb, m.boneDataCb);
			std::swap(perObjectCb, m.perObjectCb);
			std::swap(perObjectCbAddr, m.perObjectCbAddr);
		}

		Multi & operator=(Multi m) noexcept {
			std::swap(length, m.length);
			std::swap(material, m.material);
			std::swap(meshes, m.meshes);
			std::swap(boneDataCb, m.boneDataCb);
			std::swap(perObjectCb, m.perObjectCb);
			std::swap(perObjectCbAddr, m.perObjectCbAddr);
			return *this;
		}
	};

}
