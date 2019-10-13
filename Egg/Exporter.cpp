#include "Exporter.h"
#include <cstdio>
#include "Common.h"

namespace Egg {

	namespace Exporter {

		unsigned int CalculateTotalSize(const Asset::Model & m) {
			unsigned int acc = 0;

			acc += m.animationsLength * sizeof(Asset::Animation);
			for(unsigned int i = 0; i < m.animationsLength; ++i) {

				acc += sizeof(Asset::BoneAnimation) * m.animations[i].boneDataLength;

				for(unsigned j = 0; j < m.animations[i].boneDataLength; ++j) {
					acc += sizeof(Asset::AnimationKey) * m.animations[i].boneData[j].keysLength;
				}
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
				fwrite(&m.animations[i].boneDataLength, sizeof(unsigned int), 1, file);

				for(unsigned int j = 0; j < m.animations[i].boneDataLength; ++j) {
					fwrite(&(m.animations[i].boneData[j].boneId), sizeof(int), 1, file);
					fwrite(&(m.animations[i].boneData[j].preState), sizeof(Egg::Asset::AnimationState), 1, file);
					fwrite(&(m.animations[i].boneData[j].postState), sizeof(Egg::Asset::AnimationState), 1, file);
					fwrite(&(m.animations[i].boneData[j].keysLength), sizeof(unsigned int), 1, file);

					fwrite(m.animations[i].boneData[j].keys, sizeof(Asset::AnimationKey), m.animations[i].boneData[j].keysLength, file);
				}
			}

			fclose(file);
		}

	}

}

