#include "Exporter.h"
#include <cstdio>
#include "Common.h"

namespace Egg {

	namespace Exporter {

		unsigned int CalculateAnimDataSize(const Asset::Model & m) {
			unsigned int acc = 0;

			acc += sizeof(m.animationsLength);
			acc += m.animationsLength * sizeof(Asset::Animation);
			for(unsigned int i = 0; i < m.animationsLength; ++i) {
				acc += sizeof(Asset::AnimationKey) * m.animations[i].bonesLength * m.animations[i].keysLength;
				acc += sizeof(double) * m.animations[i].keysLength;
				acc += sizeof(Asset::AnimationEdge) * m.animations[i].bonesLength;
				acc += sizeof(Asset::AnimationEdge) * m.animations[i].bonesLength;
			}

			acc += sizeof(Asset::Bone) * m.bonesLength;
			return acc;
		}

		unsigned int CalculateMeshesSize(const Asset::Model & m) {
			unsigned int acc = 0;

			acc += sizeof(m.meshesLength);
			acc += sizeof(Asset::Mesh) * m.meshesLength;

			for(unsigned int i = 0; i < m.meshesLength; ++i) {
				acc += m.meshes[i].verticesLength;
				acc += m.meshes[i].indicesLength * sizeof(unsigned int);
				acc += m.meshes[i].lodLevelsLength * sizeof(Asset::LODLevel);
			}

			return acc;
		}

		unsigned int CalculateMaterialsSize(const Asset::Model & m) {
			unsigned int acc = 0;
			acc += sizeof(m.materialsLength);
			acc += sizeof(Asset::Material) * m.materialsLength;
			return acc;
		}

		unsigned int CalculateTotalSize(const Asset::Model & m) {
			return CalculateAnimDataSize(m) + CalculateMaterialsSize(m) + CalculateMeshesSize(m);
		}

		// here be dragons
		void ExportModel(const char * path, const  Asset::Model & m) {
			FILE * file;
			errno_t r = fopen_s(&file, path, "wb");

			ASSERT(r == 0, "Failed to open file for writing: %s", path);
			if(file == nullptr) {
				return;
			}
			unsigned int meshesSize = CalculateMeshesSize(m);
			unsigned int materialsSize = CalculateMaterialsSize(m);
			unsigned int animDataSize = CalculateAnimDataSize(m);

			fwrite(&meshesSize, sizeof(unsigned int), 1, file);
			fwrite(&materialsSize, sizeof(unsigned int), 1, file);
			fwrite(&animDataSize, sizeof(unsigned int), 1, file);

			size_t writtenSize = 0;
			writtenSize += sizeof(unsigned int) *fwrite(&m.meshesLength, sizeof(unsigned int), 1, file);
			writtenSize += sizeof(Asset::Mesh)* fwrite(m.meshes, sizeof(Asset::Mesh), m.meshesLength, file);

			for(unsigned int i = 0; i < m.meshesLength; ++i) {
				writtenSize += fwrite(m.meshes[i].vertices, 1, m.meshes[i].verticesLength, file);
				writtenSize += sizeof(unsigned int)* fwrite(m.meshes[i].indices, sizeof(unsigned int), m.meshes[i].indicesLength, file);
				writtenSize += sizeof(Asset::LODLevel)* fwrite(m.meshes[i].lodLevels, sizeof(Asset::LODLevel), m.meshes[i].lodLevelsLength, file);
			}

			fwrite(&m.materialsLength, sizeof(unsigned int), 1, file);
			fwrite(m.materials, sizeof(Asset::Material), m.materialsLength, file);

			fwrite(&m.animationsLength, sizeof(unsigned int), 1, file);
			fwrite(m.animations, sizeof(Asset::Animation), m.animationsLength, file);

			for(unsigned int i = 0; i < m.animationsLength; ++i) {
				fwrite(m.animations[i].preStates, sizeof(Asset::AnimationEdge), m.animations[i].bonesLength, file);
				fwrite(m.animations[i].postStates, sizeof(Asset::AnimationEdge), m.animations[i].bonesLength, file);
				fwrite(m.animations[i].times, sizeof(double), m.animations[i].keysLength, file);
				fwrite(m.animations[i].keys, sizeof(Asset::AnimationKey), m.animations[i].keysLength * m.animations[i].bonesLength, file);
			}

			fwrite(&m.bonesLength, sizeof(unsigned int), 1, file);
			fwrite(m.bones, sizeof(Asset::Bone), m.bonesLength, file);

			fclose(file);
		}

	}

}

