#pragma once

#include "Animation.h"
#include "Mesh.h"
#include "Bone.h"
#include "Material.h"
#include <memory>

namespace Egg {

	namespace Asset {

		struct Model {

			unsigned int meshesLength;
			Mesh * meshes;

			unsigned int materialsLength;
			Material * materials;

			unsigned int animationsLength;
			Animation * animations;

			unsigned int bonesLength;
			Bone * bones;

			void * memoryAllocation;

			Model() noexcept : meshesLength{ 0 }, meshes{ nullptr },
					  materialsLength{ 0 }, materials{ nullptr },
					  animationsLength{ 0 }, animations{ nullptr },
					  bonesLength{ 0 }, bones{ nullptr },
					  memoryAllocation{ nullptr } {

			}

			Model(const Model &) = delete;

			Model(Model && m) noexcept : Model() {
				std::swap(meshesLength, m.meshesLength);
				std::swap(meshes, m.meshes);

				std::swap(materialsLength, m.materialsLength);
				std::swap(materials, m.materials);

				std::swap(animationsLength, m.animationsLength);
				std::swap(animations, m.animations);

				std::swap(bonesLength, m.bonesLength);
				std::swap(bones, m.bones);

				std::swap(memoryAllocation, m.memoryAllocation);
			}

			Model & operator=(Model m) noexcept {
				std::swap(meshesLength, m.meshesLength);
				std::swap(meshes, m.meshes);

				std::swap(materialsLength, m.materialsLength);
				std::swap(materials, m.materials);

				std::swap(animationsLength, m.animationsLength);
				std::swap(animations, m.animations);

				std::swap(bonesLength, m.bonesLength);
				std::swap(bones, m.bones);

				std::swap(memoryAllocation, m.memoryAllocation);

				return *this;
			}

			~Model() noexcept {
				if(memoryAllocation) {
					std::free(memoryAllocation);
				}
			}

		};

	}

}
