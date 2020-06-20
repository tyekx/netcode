#include "Model.h"


namespace Netcode::Asset {

	namespace Detail {
		void InterpretSkip(void ** ptr, unsigned int nBytes) {
			*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(*ptr) + nBytes);
		}

		template<typename T>
		T InterpretAs(void ** ptr) {
			void * src = *ptr;
			T data = *(reinterpret_cast<T *>(src));
			*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(src) + sizeof(T));
			return data;
		}

		template<typename T>
		T * InterpretAsArray(void ** ptr, int len) {
			void * src = *ptr;
			T * data = (reinterpret_cast<T *>(src));
			*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(src) + len * sizeof(T));
			return data;
		}
	}

	Model::Model() : meshesAlloc{ nullptr, std::free },
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

	void Model::SetMaterials(void * ptr) {
		if(!ptr) {
			return;
		}

		materialsAlloc.reset(ptr);

		uint32_t len = Detail::InterpretAs<uint32_t>(&ptr);
		Material * mats = Detail::InterpretAsArray<Material>(&ptr, len);

		materials = ArrayView<Material>(mats, static_cast<size_t>(len));
	}

	void Model::SetMeshes(void * ptr) {
		if(!ptr) {
			return;
		}

		meshesAlloc.reset(ptr);

		uint32_t meshesLength = Detail::InterpretAs<unsigned int>(&ptr);
		Mesh * loadedMeshes = Detail::InterpretAsArray<Mesh>(&ptr, meshesLength);

		for(unsigned int i = 0; i < meshesLength; ++i) {
			loadedMeshes[i].vertices = ptr;
			Detail::InterpretSkip(&ptr, loadedMeshes[i].verticesSizeInBytes);
			loadedMeshes[i].indices = reinterpret_cast<uint32_t *>(ptr);
			Detail::InterpretSkip(&ptr, loadedMeshes[i].indicesSizeInBytes);
			loadedMeshes[i].lodLevelsLength = Detail::InterpretAs<uint32_t>(&ptr);
			loadedMeshes[i].lodLevels = Detail::InterpretAsArray<Asset::LODLevel>(&ptr, loadedMeshes[i].lodLevelsLength);
			loadedMeshes[i].inputElementsLength = Detail::InterpretAs<uint32_t>(&ptr);
			loadedMeshes[i].inputElements = Detail::InterpretAsArray<Asset::InputElement>(&ptr, loadedMeshes[i].inputElementsLength);
		}

		meshes = ArrayView<Mesh>(loadedMeshes, static_cast<size_t>(meshesLength));
	}

	void Model::SetAnimData(void * ptr) {
		if(!ptr) {
			return;
		}

		animDataAlloc.reset(ptr);

		uint32_t animationsLength = Detail::InterpretAs<uint32_t>(&ptr);
		Animation * loadedAnims = Detail::InterpretAsArray<Animation>(&ptr, animationsLength);

		for(unsigned int i = 0; i < animationsLength; ++i) {
			loadedAnims[i].times = Detail::InterpretAsArray<float>(&ptr, loadedAnims[i].keysLength);
			loadedAnims[i].keys = Detail::InterpretAsArray<AnimationKey>(&ptr, loadedAnims[i].keysLength * loadedAnims[i].bonesLength);
		}

		animations = ArrayView<Animation>(loadedAnims, animationsLength);
	}

	void Model::SetBoneData(void * ptr) {
		if(!ptr) {
			return;
		}

		boneDataAlloc.reset(ptr);

		uint32_t bonesLength = Detail::InterpretAs<unsigned int>(&ptr);
		Bone * loadedBones = Detail::InterpretAsArray<Bone>(&ptr, bonesLength);

		bones = ArrayView<Bone>(loadedBones, static_cast<size_t>(bonesLength));
	}

	void Model::SetColliderData(void * ptr) {
		if(!ptr) {
			return;
		}

		colliderDataAlloc.reset(ptr);

		uint32_t collidersLength = Detail::InterpretAs<uint32_t>(&ptr);
		Collider * loadedColliders = Detail::InterpretAsArray<Collider>(&ptr, collidersLength);

		colliders = ArrayView<Collider>(loadedColliders, collidersLength);
	}

}
