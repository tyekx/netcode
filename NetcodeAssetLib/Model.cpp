#include "Model.h"


namespace Egg::Asset {

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

		uint32_t len = InterpretAs<uint32_t>(&ptr);
		Material * mats = InterpretAsArray<Material>(&ptr, len);

		materials = ArrayView<Material>(mats, static_cast<size_t>(len));
	}

	void Model::SetMeshes(void * ptr) {
		if(!ptr) {
			return;
		}

		meshesAlloc.reset(ptr);

		uint32_t meshesLength = InterpretAs<unsigned int>(&ptr);
		Mesh * loadedMeshes = InterpretAsArray<Mesh>(&ptr, meshesLength);

		for(unsigned int i = 0; i < meshesLength; ++i) {
			loadedMeshes[i].vertices = ptr;
			InterpretSkip(&ptr, loadedMeshes[i].verticesSizeInBytes);
			loadedMeshes[i].indices = reinterpret_cast<uint32_t *>(ptr);
			InterpretSkip(&ptr, loadedMeshes[i].indicesSizeInBytes);
			loadedMeshes[i].lodLevelsLength = InterpretAs<uint32_t>(&ptr);
			loadedMeshes[i].lodLevels = InterpretAsArray<Asset::LODLevel>(&ptr, loadedMeshes[i].lodLevelsLength);
			loadedMeshes[i].inputElementsLength = InterpretAs<uint32_t>(&ptr);
			loadedMeshes[i].inputElements = InterpretAsArray<Asset::InputElement>(&ptr, loadedMeshes[i].inputElementsLength);
		}

		meshes = ArrayView<Mesh>(loadedMeshes, static_cast<size_t>(meshesLength));
	}

	void Model::SetAnimData(void * ptr) {
		if(!ptr) {
			return;
		}

		animDataAlloc.reset(ptr);

		uint32_t animationsLength = InterpretAs<uint32_t>(&ptr);
		Animation * loadedAnims = InterpretAsArray<Animation>(&ptr, animationsLength);

		for(unsigned int i = 0; i < animationsLength; ++i) {
			loadedAnims[i].times = InterpretAsArray<float>(&ptr, loadedAnims[i].keysLength);
			loadedAnims[i].keys = InterpretAsArray<AnimationKey>(&ptr, loadedAnims[i].keysLength * loadedAnims[i].bonesLength);
		}

		animations = ArrayView<Animation>(loadedAnims, animationsLength);
	}

	void Model::SetBoneData(void * ptr) {
		if(!ptr) {
			return;
		}

		boneDataAlloc.reset(ptr);

		uint32_t bonesLength = InterpretAs<unsigned int>(&ptr);
		Bone * loadedBones = InterpretAsArray<Bone>(&ptr, bonesLength);

		bones = ArrayView<Bone>(loadedBones, static_cast<size_t>(bonesLength));
	}

	void Model::SetColliderData(void * ptr) {
		if(!ptr) {
			return;
		}

		colliderDataAlloc.reset(ptr);

		uint32_t collidersLength = InterpretAs<uint32_t>(&ptr);
		Collider * loadedColliders = InterpretAsArray<Collider>(&ptr, collidersLength);

		colliders = ArrayView<Collider>(loadedColliders, collidersLength);
	}

}
