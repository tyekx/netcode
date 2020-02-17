#pragma once

#include <memory>
#include <EggFoundation/Foundation.h>

#include "Animation.h"
#include "Mesh.h"
#include "Bone.h"
#include "Material.h"
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

		Model();

		void SetMaterials(void * ptr);

		void SetMeshes(void * ptr);

		void SetAnimData(void * ptr);

		void SetBoneData(void * ptr);
		
		void SetColliderData(void * ptr);
	};
}
