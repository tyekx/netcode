#include "Importer.h"

#include "Utility.h"
#include "Vertex.h"
#include "LinearAllocator.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <system_error>
#include <cstdio>

#include <DirectXTex/DirectXTex.h>

namespace Egg::Importer {

	Graphics::Resource::Committed::Texture2D  ImportCommittedTexture2D(ID3D12Device * device, const MediaPath & filePath) {
		std::wstring wstr = filePath.GetAbsolutePath();

		DirectX::TexMetadata metaData;
		DirectX::ScratchImage sImage;
		DirectX::ScratchImage outputIm;

		DX_API("Failed to load image: %S", wstr.c_str())
			DirectX::LoadFromWICFile(wstr.c_str(), 0, &metaData, sImage);

		bool isPow2 = Egg::Utility::IsPowerOf2((unsigned int)metaData.width);

		//DirectX::GenerateMipMaps(sImage.GetImages(), sImage.GetImageCount(), sImage.GetMetadata(), DirectX::TEX_FILTER_BOX, 3, outputIm);

		D3D12_RESOURCE_DESC rdsc;
		ZeroMemory(&rdsc, sizeof(D3D12_RESOURCE_DESC));
		rdsc.DepthOrArraySize = 1;
		rdsc.Height = (unsigned int)metaData.height;
		rdsc.Width = (unsigned int)metaData.width;
		rdsc.Format = metaData.format;
		rdsc.MipLevels = 1;
		rdsc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rdsc.Alignment = 0;
		rdsc.SampleDesc.Count = 1;
		rdsc.SampleDesc.Quality = 0;
		rdsc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rdsc.Flags = D3D12_RESOURCE_FLAG_NONE;

		Graphics::Resource::Committed::Texture2D texture;
		texture.CreateResources(device, rdsc, sImage.GetPixels(), sImage.GetPixelsSize());
		return texture;
	}

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

		unsigned int totalSize;
		fread(&totalSize, sizeof(unsigned int), 1, file);

		Memory::LinearAllocator allocator{ totalSize };

		fread(&m.meshesLength, sizeof(unsigned int), 1, file);

		m.meshes = reinterpret_cast<Asset::Mesh *>(allocator.Allocate(m.meshesLength * sizeof(Asset::Mesh)));

		for(unsigned int i = 0; i < m.meshesLength; ++i) {
			fread(&m.meshes[i].materialId, sizeof(unsigned int), 1, file);
			fread(&m.meshes[i].vertexType, sizeof(unsigned int), 1, file);
			fread(&m.meshes[i].vertexSize, sizeof(unsigned int), 1, file);
			fread(&m.meshes[i].verticesLength, sizeof(unsigned int), 1, file);

			m.meshes[i].vertices = allocator.Allocate(m.meshes[i].verticesLength);

			fread(m.meshes[i].vertices, 1, m.meshes[i].verticesLength, file);

			fread(&m.meshes[i].indicesLength, sizeof(unsigned int), 1, file);

			m.meshes[i].indices = reinterpret_cast<unsigned int *>(allocator.Allocate(m.meshes[i].indicesLength * sizeof(unsigned int)));

			fread(m.meshes[i].indices, sizeof(unsigned int), m.meshes[i].indicesLength, file);
		}

		fread(&m.materialsLength, sizeof(unsigned int), 1, file);
		m.materials = reinterpret_cast<Asset::Material *>(allocator.Allocate(m.materialsLength * sizeof(Asset::Material)));
		fread(m.materials, sizeof(Asset::Material), m.materialsLength, file);

		fread(&m.bonesLength, sizeof(unsigned int), 1, file);
		m.bones = reinterpret_cast<Asset::Bone *>(allocator.Allocate(m.bonesLength * sizeof(Asset::Bone)));
		fread(m.bones, sizeof(Asset::Bone), m.bonesLength, file);

		fread(&m.animationsLength, sizeof(unsigned int), 1, file);
		m.animations = reinterpret_cast<Asset::Animation *>(allocator.Allocate(m.animationsLength * sizeof(Asset::Animation)));
		for(unsigned int i = 0; i < m.animationsLength; ++i) {
			fread(m.animations[i].name, 1, sizeof(m.animations[i].name), file);
			fread(&m.animations[i].duration, sizeof(double), 1, file);
			fread(&m.animations[i].ticksPerSecond, sizeof(double), 1, file);
			fread(&m.animations[i].keysLength, sizeof(unsigned int), 1, file);
			fread(&m.animations[i].bonesLength, sizeof(unsigned int), 1, file);

			m.animations[i].preStates = reinterpret_cast<Asset::AnimationEdge *>(allocator.Allocate(m.animations[i].bonesLength * sizeof(Asset::AnimationEdge)));
			m.animations[i].postStates = reinterpret_cast<Asset::AnimationEdge *>(allocator.Allocate(m.animations[i].bonesLength * sizeof(Asset::AnimationEdge)));
			m.animations[i].times = reinterpret_cast<double *>(allocator.Allocate(m.animations[i].keysLength * sizeof(double)));
			m.animations[i].keys = reinterpret_cast<Asset::AnimationKey *>(allocator.Allocate(m.animations[i].bonesLength * m.animations[i].keysLength * sizeof(Asset::AnimationKey)));

			fread(m.animations[i].preStates, sizeof(Asset::AnimationEdge), m.animations[i].bonesLength, file);
			fread(m.animations[i].postStates, sizeof(Asset::AnimationEdge), m.animations[i].bonesLength, file);
			fread(m.animations[i].times, sizeof(double), m.animations[i].keysLength, file);
			fread(m.animations[i].keys, sizeof(Asset::AnimationKey), m.animations[i].keysLength * m.animations[i].bonesLength, file);
		}

		fclose(file);

		m.memoryAllocation = allocator.Detach();

	}
}
