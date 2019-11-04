#pragma once

#include "Material.h"
#include "Mesh.h"

namespace Egg {

	struct Model {
		UINT gpuResourcesHandle;

		UINT meshesLength;
		PerObjectCb * perObjectCb;
		BoneDataCb * boneDataCb;
		MaterialCb ** materials;

		void InitWithLength(UINT len) {
			meshesLength = len;
			materials = reinterpret_cast<MaterialCb **>(std::malloc(len * sizeof(MaterialCb *)));
			memset(materials, 0, len * sizeof(MaterialCb *));
		}

		Model(UINT len) : Model{} {
			InitWithLength(len);
		}

		Model() :gpuResourcesHandle{ 0 }, meshesLength{ 0 }, perObjectCb{ nullptr }, boneDataCb{ nullptr }, materials{ nullptr } { }
		~Model() = default;

		Model(const Model & m) : Model{} {
			InitWithLength(m.meshesLength);
			boneDataCb = m.boneDataCb;
			perObjectCb = m.perObjectCb;
			for(UINT i = 0; i < meshesLength; ++i) {
				materials[i] = m.materials[i];
			}
		}

		Model(Model && m) noexcept : Model{} {
			std::swap(meshesLength, m.meshesLength);
			std::swap(materials, m.materials);
			std::swap(boneDataCb, m.boneDataCb);
			std::swap(perObjectCb, m.perObjectCb);
		}

		Model & operator=(Model m) noexcept {
			std::swap(meshesLength, m.meshesLength);
			std::swap(materials, m.materials);
			std::swap(boneDataCb, m.boneDataCb);
			std::swap(perObjectCb, m.perObjectCb);
			return *this;
		}
	};

}
