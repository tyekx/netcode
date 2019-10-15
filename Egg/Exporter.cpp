#include "Exporter.h"
#include <cstdio>
#include "Common.h"

namespace Egg {

	namespace Exporter {

		unsigned int CalculateTotalSize(const Asset::Model & m) {
			unsigned int acc = 0;

			acc += m.animationsLength * sizeof(Asset::Animation);
			for(unsigned int i = 0; i < m.animationsLength; ++i) {
				acc += sizeof(Asset::AnimationKey) * m.animations[i].bonesLength * m.animations[i].keysLength;
				acc += sizeof(double) * m.animations[i].keysLength;
				acc += sizeof(Asset::AnimationState) * m.animations[i].bonesLength;
				acc += sizeof(Asset::AnimationState) * m.animations[i].bonesLength;
			}

			acc += sizeof(Asset::Bone) * m.bonesLength;
			acc += sizeof(Asset::Material) * m.materialsLength;

			acc += sizeof(Asset::Mesh) * m.meshesLength;

			for(unsigned int i = 0; i < m.meshesLength; ++i) {
				acc += m.meshes[i].verticesLength;
				acc += m.meshes[i].indicesLength * sizeof(unsigned int);
			}

			return acc;
		}

		// here be dragons
		void ExportModel(const char * path, const  Asset::Model & m) {
			FILE * file;
			errno_t r = fopen_s(&file, path, "wb");

			ASSERT(r == 0, "Failed to open file for writing: %s", path);
			if(file == nullptr) {
				return;
			}

			unsigned int totalSize = CalculateTotalSize(m);

			fwrite(&totalSize, sizeof(unsigned int), 1, file);
			fwrite(&m.meshesLength, sizeof(unsigned int), 1, file);

			for(unsigned int i = 0; i < m.meshesLength; ++i) {
				fwrite(&m.meshes[i].materialId, sizeof(unsigned int), 1, file);
				fwrite(&m.meshes[i].vertexType, sizeof(unsigned int), 1, file);
				fwrite(&m.meshes[i].vertexSize, sizeof(unsigned int), 1, file);
				fwrite(&m.meshes[i].verticesLength, sizeof(unsigned int), 1, file);
				fwrite(m.meshes[i].vertices, 1, m.meshes[i].verticesLength, file);
				fwrite(&m.meshes[i].indicesLength, sizeof(unsigned int), 1, file);
				fwrite(m.meshes[i].indices, sizeof(unsigned int), m.meshes[i].indicesLength, file);
			}

			fwrite(&m.materialsLength, sizeof(unsigned int), 1, file);
			fwrite(m.materials, sizeof(Asset::Material), m.materialsLength, file);

			fwrite(&m.bonesLength, sizeof(unsigned int), 1, file);
			fwrite(m.bones, sizeof(Asset::Bone), m.bonesLength, file);

			fwrite(&m.animationsLength, sizeof(unsigned int), 1, file);
			for(unsigned int i = 0; i < m.animationsLength; ++i) {
				fwrite(m.animations[i].name, 1, sizeof(m.animations[i].name), file);
				fwrite(&m.animations[i].duration, sizeof(double), 1, file);
				fwrite(&m.animations[i].ticksPerSecond, sizeof(double), 1, file);
				fwrite(&m.animations[i].keysLength, sizeof(unsigned int), 1, file);
				fwrite(&m.animations[i].bonesLength, sizeof(unsigned int), 1, file);

				fwrite(m.animations[i].preStates, sizeof(Asset::AnimationState), m.animations[i].bonesLength, file);
				fwrite(m.animations[i].postStates, sizeof(Asset::AnimationState), m.animations[i].bonesLength, file);
				fwrite(m.animations[i].times, sizeof(double), m.animations[i].keysLength, file);
				fwrite(m.animations[i].keys, sizeof(Asset::AnimationKey), m.animations[i].keysLength * m.animations[i].bonesLength, file);
			}

			fclose(file);
		}

	}

}

