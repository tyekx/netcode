#include "Exporter.h"
#include <cstdio>
#include "Common.h"

namespace Egg {

	namespace Exporter {

		uint32_t CalculateAnimDataSize(const Asset::Model & m) {
			uint32_t acc = 0;

			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(m.animations.Size() * sizeof(Asset::Animation));
			for(size_t i = 0; i < m.animations.Size(); ++i) {
				acc += sizeof(Asset::AnimationKey) * m.animations[i].bonesLength * m.animations[i].keysLength;
				acc += sizeof(float) * m.animations[i].keysLength;
			}

			return acc;
		}

		uint32_t CalculateMeshesSize(const Asset::Model & m) {
			uint32_t acc = 0;

			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(sizeof(Asset::Mesh) * m.meshes.Size());

			for(uint32_t i = 0; i < m.meshes.Size(); ++i) {
				acc += m.meshes[i].verticesLength;
				acc += m.meshes[i].indicesLength * sizeof(uint32_t);
				acc += m.meshes[i].lodLevelsLength * sizeof(Asset::LODLevel);
			}

			return acc;
		}

		uint32_t CalculateMaterialsSize(const Asset::Model & m) {
			uint32_t acc = 0;
			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(sizeof(Asset::Material) * m.materials.Size());
			return acc;
		}

		uint32_t CalculateCollidersSize(const Asset::Model & m) {
			uint32_t acc = 0;
			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(sizeof(Asset::Collider) * m.colliders.Size());
			return acc;
		}

		uint32_t CalculateBonesSize(const Asset::Model & m) {
			uint32_t acc = 0;
			acc += sizeof(uint32_t);
			acc += static_cast<uint32_t>(sizeof(Asset::Bone) * m.bones.Size());
			return acc;
		}

		uint32_t CalculateTotalSize(const Asset::Model & m) {
			return CalculateAnimDataSize(m) + CalculateMaterialsSize(m) + CalculateMeshesSize(m) + CalculateCollidersSize(m) + CalculateBonesSize(m);
		}

		// here be dragons
		uint32_t ExportModel(const char * path, const  Asset::Model & m) {
			FILE * file;
			errno_t r = fopen_s(&file, path, "wb");

			ASSERT(r == 0, "Failed to open file for writing: %s", path);
			if(file == nullptr) {
				return;
			}

			uint32_t meshesSize = CalculateMeshesSize(m);
			uint32_t materialsSize = CalculateMaterialsSize(m);
			uint32_t animDataSize = CalculateAnimDataSize(m);
			uint32_t colliderDataSize = CalculateCollidersSize(m);
			uint32_t boneDataSize = CalculateBonesSize(m);

			size_t writtenSize = 0;

			sizeof(uint32_t) * fwrite(&meshesSize, sizeof(uint32_t), 1, file);
			sizeof(uint32_t) * fwrite(&animDataSize, sizeof(uint32_t), 1, file);
			sizeof(uint32_t) * fwrite(&materialsSize, sizeof(uint32_t), 1, file);
			sizeof(uint32_t) * fwrite(&boneDataSize, sizeof(uint32_t), 1, file);
			sizeof(uint32_t) * fwrite(&colliderDataSize, sizeof(uint32_t), 1, file);

			uint32_t meshesLength = static_cast<uint32_t>(m.meshes.Size());
			uint32_t materialsLength = static_cast<uint32_t>(m.materials.Size());
			uint32_t animsLength = static_cast<uint32_t>(m.animations.Size());
			uint32_t bonesLength = static_cast<uint32_t>(m.bones.Size());
			uint32_t collidersLength = static_cast<uint32_t>(m.colliders.Size());

			writtenSize += sizeof(uint32_t) * fwrite(&meshesLength, sizeof(uint32_t), 1, file);
			writtenSize += sizeof(Asset::Mesh) * fwrite(m.meshes.Data(), sizeof(Asset::Mesh), meshesLength, file);

			for(uint32_t i = 0; i < meshesLength; ++i) {
				writtenSize += fwrite(m.meshes[i].vertices, 1, m.meshes[i].verticesLength, file);
				writtenSize += sizeof(uint32_t) * fwrite(m.meshes[i].indices, sizeof(uint32_t), m.meshes[i].indicesLength, file);
				writtenSize += sizeof(Asset::LODLevel) * fwrite(m.meshes[i].lodLevels, sizeof(Asset::LODLevel), m.meshes[i].lodLevelsLength, file);
			}

			writtenSize += sizeof(uint32_t) * fwrite(&animsLength, sizeof(uint32_t), 1, file);
			writtenSize += sizeof(Asset::Animation) * fwrite(m.animations.Data(), sizeof(Asset::Animation), animsLength, file);

			for(uint32_t i = 0; i < animsLength; ++i) {
				writtenSize += sizeof(float) * fwrite(m.animations[i].times, sizeof(float), m.animations[i].keysLength, file);
				writtenSize += sizeof(Asset::AnimationKey) * fwrite(m.animations[i].keys, sizeof(Asset::AnimationKey), m.animations[i].keysLength * m.animations[i].bonesLength, file);
			}

			writtenSize += sizeof(uint32_t) * fwrite(&materialsLength, sizeof(uint32_t), 1, file);
			writtenSize += sizeof(Asset::Material) * fwrite(m.materials.Data(), sizeof(Asset::Material), materialsLength, file);

			writtenSize += sizeof(uint32_t) * fwrite(&bonesLength, sizeof(uint32_t), 1, file);
			writtenSize += sizeof(Asset::Bone) * fwrite(m.bones.Data(), sizeof(Asset::Bone), bonesLength, file);

			writtenSize += sizeof(uint32_t) * fwrite(&collidersLength, sizeof(uint32_t), 1, file);
			writtenSize += sizeof(Asset::Collider) * fwrite(m.colliders.Data(), sizeof(Asset::Collider), collidersLength, file);

			fclose(file);

			return writtenSize;
		}

	}

}

