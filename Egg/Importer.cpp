#include "Importer.h"

#include "Utility.h"
#include "Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <system_error>

#include <DirectXTex/DirectXTex.h>

namespace Egg {
	namespace Importer {
		Texture2D ImportTexture2D(ID3D12Device * device, const MediaPath & filePath) {
			std::wstring wstr = filePath.GetAbsolutePath();

			DirectX::TexMetadata metaData;
			DirectX::ScratchImage sImage;

			DX_API("Failed to load image: %S", wstr.c_str())
				DirectX::LoadFromWICFile(wstr.c_str(), 0, &metaData, sImage);

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

			com_ptr<ID3D12Resource> resource;
			com_ptr<ID3D12Resource> uploadResource;

			DX_API("failed to create committed resource for texture file")
				device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&rdsc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(resource.GetAddressOf()));

			UINT64 copyableSize;
			device->GetCopyableFootprints(&rdsc, 0, 1, 0, nullptr, nullptr, nullptr, &copyableSize);

			DX_API("failed to create committed resource for texture file (upload buffer)")
				device->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
					D3D12_HEAP_FLAG_NONE,
					&CD3DX12_RESOURCE_DESC::Buffer(copyableSize),
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(uploadResource.GetAddressOf()));

			CD3DX12_RANGE readRange{ 0,0 };
			void * mappedPtr;

			DX_API("Failed to map upload resource")
				uploadResource->Map(0, &readRange, &mappedPtr);

			memcpy(mappedPtr, sImage.GetPixels(), sImage.GetPixelsSize());

			uploadResource->Unmap(0, nullptr);

			return Texture2D{ std::move(resource), std::move(uploadResource), rdsc };
		}

		Egg::Mesh::Geometry::P ImportSimpleObj(ID3D12Device * device, const MediaPath & filePath) {
			std::string path = Egg::Utility::ToNarrowString(filePath.GetAbsolutePath());

			Assimp::Importer importer;

			const aiScene * scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_GenUVCoords);

			ASSERT(scene != nullptr, "Failed to load obj file: '%s'. Assimp error message: '%s'", path.c_str(), importer.GetErrorString());

			ASSERT(scene->HasMeshes(), "Obj file: '%s' does not contain a mesh.", path.c_str());

			// for this example we only load the first mesh
			const aiMesh * mesh = scene->mMeshes[0];

			std::vector<unsigned int> indices;
			std::vector<PNT_Vertex> vertices;
			indices.reserve(mesh->mNumFaces);
			vertices.reserve(mesh->mNumVertices);

			PNT_Vertex v;

			for(unsigned int i = 0; i < mesh->mNumVertices; ++i) {
				v.position.x = mesh->mVertices[i].x;
				v.position.y = mesh->mVertices[i].y;
				v.position.z = mesh->mVertices[i].z;

				v.normal.x = mesh->mNormals[i].x;
				v.normal.y = mesh->mNormals[i].y;
				v.normal.z = mesh->mNormals[i].z;

				v.tex.x = mesh->mTextureCoords[0][i].x;
				v.tex.y = mesh->mTextureCoords[0][i].y;

				vertices.emplace_back(v);
			}

			for(unsigned int i = 0; i < mesh->mNumFaces; ++i) {
				aiFace face = mesh->mFaces[i];
				indices.emplace_back(face.mIndices[0]);
				indices.emplace_back(face.mIndices[1]);
				indices.emplace_back(face.mIndices[2]);
			}

			Egg::Mesh::Geometry::P geometry = Egg::Mesh::IndexedGeometry::Create(device, &(vertices.at(0)), (unsigned int)(vertices.size() * sizeof(PNT_Vertex)), (unsigned int)sizeof(PNT_Vertex),
																				 &(indices.at(0)), (unsigned int)(indices.size() * 4), DXGI_FORMAT_R32_UINT);

			geometry->AddInputElement({ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			geometry->AddInputElement({ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
			geometry->AddInputElement({ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

			return geometry;
		}
	}
}
