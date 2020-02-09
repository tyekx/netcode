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
		freed_unique_ptr_t colliderDataAlloc;
	public:
		unsigned int meshesLength;
		Mesh * meshes;

		unsigned int materialsLength;
		Material * materials;

		unsigned int animationsLength;
		Animation * animations;

		unsigned int bonesLength;
		Bone * bones;

		Model() : meshesAlloc{ nullptr, std::free }, materialsAlloc{ nullptr, std::free }, animDataAlloc{ nullptr, std::free },
			colliderDataAlloc{ nullptr, std::free },
			meshesLength{ 0 }, meshes{ nullptr }, 
			materialsLength{ 0 }, materials{ nullptr },
			animationsLength{ 0 }, animations{ nullptr },
			bonesLength{ 0 }, bones{ nullptr } {}

		void SetMaterials(void * ptr) {
			if(!ptr) {
				return;
			}

			materialsAlloc.reset(ptr);

			materialsLength = InterpretAs<unsigned int>(&ptr);
			materials = InterpretAsArray<Material>(&ptr, materialsLength);
		}

		void SetMeshes(void * ptr) {
			if(!ptr) {
				return;
			}

			meshesAlloc.reset(ptr);

			meshesLength = InterpretAs<unsigned int>(&ptr);
			meshes = InterpretAsArray<Mesh>(&ptr, meshesLength);
			
			for(unsigned int i = 0; i < meshesLength; ++i) {
				meshes[i].vertices = ptr;
				InterpretSkip(&ptr, meshes[i].verticesLength);
				meshes[i].indices = InterpretAsArray<unsigned int>(&ptr, meshes[i].indicesLength);
				meshes[i].lodLevels = InterpretAsArray<Asset::LODLevel>(&ptr, meshes[i].lodLevelsLength);
			}
		}

		// contains bones too
		void SetAnimData(void * ptr) {
			if(!ptr) {
				return;
			}

			animDataAlloc.reset(ptr);

			animationsLength = InterpretAs<unsigned int>(&ptr);
			animations = InterpretAsArray<Animation>(&ptr, animationsLength);

			for(unsigned int i = 0; i < animationsLength; ++i) {
				animations[i].times = InterpretAsArray<float>(&ptr, animations[i].keysLength);
				animations[i].keys = InterpretAsArray<AnimationKey>(&ptr, animations[i].keysLength * animations[i].bonesLength);
			}

			bonesLength = InterpretAs<unsigned int>(&ptr);
			bones = InterpretAsArray<Bone>(&ptr, bonesLength);
		}
		
		void SetColliderData(void * ptr) {

		}
	};
}
