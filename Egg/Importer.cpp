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

	std::unique_ptr<Graphics::DX12::Resource::ITexture>  ImportCommittedTexture2D(ID3D12Device * device, const MediaPath & filePath) {
		std::wstring wstr = filePath.GetAbsolutePath();

		DirectX::TexMetadata metaData;
		DirectX::ScratchImage sImage;
		DirectX::ScratchImage outputIm;

		DX_API("Failed to load image: %S", wstr.c_str())
			DirectX::LoadFromWICFile(wstr.c_str(), 0, &metaData, sImage);

		bool isPow2 = Egg::Utility::IsPowerOf2((unsigned int)metaData.width);

		ASSERT(isPow2, "texture resolution must be power of 2");

		ASSERT(metaData.width >= 512 && metaData.width == metaData.height, "Invalid texture parameters");

		DX_API("Failed to generate mip levels")
		DirectX::GenerateMipMaps(sImage.GetImages(), sImage.GetImageCount(), sImage.GetMetadata(), DirectX::TEX_FILTER_BOX, 8, outputIm);

		metaData = outputIm.GetMetadata();

		D3D12_RESOURCE_DESC rdsc;
		ZeroMemory(&rdsc, sizeof(D3D12_RESOURCE_DESC));
		rdsc.DepthOrArraySize = 1;
		rdsc.Height = (unsigned int)metaData.height;
		rdsc.Width = (unsigned int)metaData.width;
		rdsc.Format = metaData.format;
		rdsc.MipLevels = static_cast<UINT16>(metaData.mipLevels);
		rdsc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rdsc.Alignment = 0;
		rdsc.SampleDesc.Count = 1;
		rdsc.SampleDesc.Quality = 0;
		rdsc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rdsc.Flags = D3D12_RESOURCE_FLAG_NONE;

		std::unique_ptr<Graphics::DX12::Resource::CommittedTexture2D> texture = std::make_unique<Graphics::DX12::Resource::CommittedTexture2D>();
		texture->CreateResources(device, rdsc, std::move(outputIm));
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

		unsigned int meshesSize;
		unsigned int materialsSize;
		unsigned int animDataSize;

		fread(&meshesSize, sizeof(unsigned int), 1, file);
		fread(&materialsSize, sizeof(unsigned int), 1, file);
		fread(&animDataSize, sizeof(unsigned int), 1, file);

		void * meshesData = nullptr;
		void * materialsData = nullptr;
		void * animData = nullptr;

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

		m.SetMeshes(meshesData);
		m.SetMaterials(materialsData);
		m.SetAnimData(animData);

		fclose(file);

		/*
		Post processing: deleting every unresolved texture references
		*/
		for(unsigned int i = 0; i < m.materialsLength; ++i) {
			if(m.materials[i].HasDiffuseTexture()) {
				MediaPath mp{ Egg::Utility::ToWideString(m.materials[i].diffuseTexture) };
				if(!Egg::Path::FileExists(mp.GetAbsolutePath().c_str())) {
					ZeroMemory(m.materials[i].diffuseTexture, sizeof(m.materials[i].diffuseTexture));
				}
			}
			if(m.materials[i].HasNormalTexture()) {
				MediaPath mp{ Egg::Utility::ToWideString(m.materials[i].normalTexture) };
				if(!Egg::Path::FileExists(mp.GetAbsolutePath().c_str())) {
					ZeroMemory(m.materials[i].normalTexture, sizeof(m.materials[i].normalTexture));
				}
			}
		}

	}
}
