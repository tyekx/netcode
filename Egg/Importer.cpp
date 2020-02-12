#include "Importer.h"

#include "Utility.h"
#include "Vertex.h"
#include "LinearClassifier.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <system_error>
#include <cstdio>

#include <DirectXTex/DirectXTex.h>

namespace Egg::Importer {

	void ImportModel(const MediaPath & mediaFile, Asset::Model & model) {
		std::string mediaFilePath = Egg::Utility::ToNarrowString(mediaFile.GetAbsolutePath());
		ImportModel(mediaFilePath.c_str(), model);
	}

	void ImportModel(const char * filePath, Asset::Model & m) {
		std::string path = filePath;

		FILE * file;
		errno_t r = fopen_s(&file, path.c_str(), "rb");

		ASSERT(r == 0, "Failed to open file for reading: %s", path.c_str());

		if(file == nullptr) {
			return;
		}

		uint32_t meshesSize;
		uint32_t materialsSize;
		uint32_t animDataSize;
		uint32_t colliderDataSize;
		uint32_t boneDataSize;

		fread(&meshesSize, sizeof(uint32_t), 1, file);
		fread(&animDataSize, sizeof(uint32_t), 1, file);
		fread(&materialsSize, sizeof(uint32_t), 1, file);
		fread(&boneDataSize, sizeof(uint32_t), 1, file);
		fread(&colliderDataSize, sizeof(uint32_t), 1, file);

		void * meshesData = nullptr;
		void * materialsData = nullptr;
		void * animData = nullptr;
		void * colliderData = nullptr;
		void * boneData = nullptr;

		if(meshesSize > 0) {
			meshesData = std::malloc(meshesSize);
			fread(meshesData, 1, meshesSize, file);
		}

		if(materialsSize > 0) {
			materialsData = std::malloc(materialsSize);
			fread(materialsData, 1, materialsSize, file);
		}

		if(animDataSize > 0) {
			animData = std::malloc(animDataSize);
			fread(animData, 1, animDataSize, file);
		}

		if(boneDataSize > 0) {
			boneData = std::malloc(boneDataSize);
			fread(boneData, 1, boneDataSize, file);
		}

		if(colliderDataSize > 0) {
			colliderData = std::malloc(colliderDataSize);
			fread(colliderData, 1, colliderDataSize, file);
		}

		m.SetMeshes(meshesData);
		m.SetMaterials(materialsData);
		m.SetAnimData(animData);
		m.SetColliderData(colliderData);
		m.SetBoneData(boneData);

		fclose(file);
	}
}
