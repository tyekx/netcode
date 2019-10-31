#pragma once

#include "Material.h"
#include "Mesh.h"

namespace Egg {

	struct Multi {
		UINT length;
		std::unique_ptr<Material[]> material;
		std::unique_ptr<Mesh[]> meshes;
		PerObjectCb * perObjectCb;
		D3D12_GPU_VIRTUAL_ADDRESS perObjectCbAddr;

		void InitWithLength(UINT len) {
			length = len;
			material = std::make_unique<Material[]>(length);
			meshes = std::make_unique<Mesh[]>(length);
		}

		inline void Draw(ID3D12GraphicsCommandList * gcl) {
			for(UINT i = 0; i < length; ++i) {
				material[i].SetPipelineState(gcl);
				int perObjCbId = material[i].cbAssoc[PerObjectCb::id];
				if(perObjCbId != -1) {
					gcl->SetGraphicsRootConstantBufferView((UINT)perObjCbId, perObjectCbAddr);
				}
				meshes[i].Draw(gcl);
			}
		}

		Multi(UINT len) : Multi{} {
			InitWithLength(len);
		}

		Multi() : length{ 0 }, material{ nullptr }, meshes{ nullptr }, perObjectCb{ nullptr }, perObjectCbAddr{ 0 } { }
		~Multi() = default;

		Multi(const Multi & m) : Multi{} {
			InitWithLength(m.length);

			for(UINT i = 0; i < length; ++i) {
				meshes[i] = m.meshes[i];
				material[i] = m.material[i];
			}
		}

		Multi(Multi && m) noexcept : Multi{} {
			std::swap(length, m.length);
			std::swap(material, m.material);
			std::swap(meshes, m.meshes);
		}

		Multi & operator=(Multi m) noexcept {
			std::swap(length, m.length);
			std::swap(material, m.material);
			std::swap(meshes, m.meshes);
			return *this;
		}
	};

}
