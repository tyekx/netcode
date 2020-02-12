#pragma once

#include "Animation.h"
#include "Mesh.h"
#include "Bone.h"
#include "Material.h"
#include <memory>
#include "../Common.h"
#include "../MemoryFunctions.h"
#include "Collider.h"

namespace Egg::Asset {



	class Model {
		using freed_unique_ptr_t = std::unique_ptr<void, void(*)(void *)>;

		freed_unique_ptr_t meshesAlloc;
		freed_unique_ptr_t materialsAlloc;
		freed_unique_ptr_t animDataAlloc;
		freed_unique_ptr_t boneDataAlloc;
		freed_unique_ptr_t colliderDataAlloc;
	public:

		ArrayView<Mesh> meshes;
		ArrayView<Material> materials;
		ArrayView<Animation> animations;
		ArrayView<Bone> bones;
		ArrayView<Collider> colliders;


		Model() : meshesAlloc{ nullptr, std::free },
			materialsAlloc{ nullptr, std::free },
			animDataAlloc{ nullptr, std::free },
			boneDataAlloc{ nullptr, std::free },
			colliderDataAlloc{ nullptr, std::free },
			meshes{},
			materials{},
			animations{},
			bones{},
			colliders{} {
		
		}

		void SetMaterials(void * ptr) {
			if(!ptr) {
				return;
			}

			materialsAlloc.reset(ptr);

			uint32_t len = InterpretAs<uint32_t>(&ptr);
			Material* mats = InterpretAsArray<Material>(&ptr, len);

			materials = ArrayView<Material>(mats, static_cast<size_t>(len));
		}

		void SetMeshes(void * ptr) {
			if(!ptr) {
				return;
			}

			meshesAlloc.reset(ptr);

			uint32_t meshesLength = InterpretAs<unsigned int>(&ptr);
			Mesh* loadedMeshes = InterpretAsArray<Mesh>(&ptr, meshesLength);
			
			for(unsigned int i = 0; i < meshesLength; ++i) {
				loadedMeshes[i].vertices = ptr;
				InterpretSkip(&ptr, loadedMeshes[i].verticesSizeInBytes);
				loadedMeshes[i].indices = reinterpret_cast<uint32_t *>(ptr);
				InterpretSkip(&ptr, loadedMeshes[i].indicesSizeInBytes);
				loadedMeshes[i].lodLevels = InterpretAsArray<Asset::LODLevel>(&ptr, loadedMeshes[i].lodLevelsLength);
			}

			meshes = ArrayView<Mesh>(loadedMeshes, static_cast<size_t>(meshesLength));
		}

		void SetAnimData(void * ptr) {
			if(!ptr) {
				return;
			}

			animDataAlloc.reset(ptr);

			uint32_t animationsLength = InterpretAs<uint32_t>(&ptr);
			Animation* loadedAnims = InterpretAsArray<Animation>(&ptr, animationsLength);

			for(unsigned int i = 0; i < animationsLength; ++i) {
				loadedAnims[i].times = InterpretAsArray<float>(&ptr, loadedAnims[i].keysLength);
				loadedAnims[i].keys = InterpretAsArray<AnimationKey>(&ptr, loadedAnims[i].keysLength * loadedAnims[i].bonesLength);
			}

			animations = ArrayView<Animation>(loadedAnims, animationsLength);
		}

		void SetBoneData(void * ptr) {
			if(!ptr) {
				return;
			}

			boneDataAlloc.reset(ptr);

			uint32_t bonesLength = InterpretAs<unsigned int>(&ptr);
			Bone * loadedBones = InterpretAsArray<Bone>(&ptr, bonesLength);

			bones = ArrayView<Bone>(loadedBones, static_cast<size_t>(bonesLength));
		}
		
		void SetColliderData(void * ptr) {
			if(!ptr) {
				return;
			}

			colliderDataAlloc.reset(ptr);

			uint32_t collidersLength = InterpretAs<uint32_t>(&ptr);
			Collider * loadedColliders = InterpretAsArray<Collider>(&ptr, collidersLength);

			colliders = ArrayView<Collider>(loadedColliders, collidersLength);
		}
	};
}
