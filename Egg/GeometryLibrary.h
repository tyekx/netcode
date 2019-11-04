#pragma once

#include "Asset/Mesh.h"
#include "Geometry.h"
#include "CommittedIBuffer.h"
#include "CommittedVBuffer.h"


namespace Egg::Graphics::Internal {

	class GeometryLibrary : Resource::IUploadResource {

		ID3D12Device * device;

		struct StorageItem {
			Asset::Mesh * meshRef;
			Geometry geometry;
		};

		std::vector<StorageItem> items;


		bool Exist(Asset::Mesh * mesh) {
			for(auto & i : items) {
				if(i.meshRef == mesh) {
					return true;
				}
			}
			return false;
		}

		void CreateGeometry(Asset::Mesh * mesh) {
			StorageItem si;

			std::unique_ptr<Resource::Committed::VBuffer> vbuffer = std::make_unique<Resource::Committed::VBuffer>();
			vbuffer->CreateResources(device, CD3DX12_RESOURCE_DESC::Buffer(mesh->verticesLength), mesh->vertices, mesh->verticesLength, mesh->vertexSize);
			//vbuffer->UploadResources(gcl);

			std::unique_ptr<Resource::Committed::IBuffer> ibuffer{ nullptr };

			if(mesh->indices != nullptr) {
				ibuffer = std::make_unique<Resource::Committed::IBuffer>();



				ibuffer->CreateResources(device, CD3DX12_RESOURCE_DESC::Buffer(mesh->indicesLength * 4U), mesh->indices, mesh->indicesLength * 4U, DXGI_FORMAT_R32_UINT);
				//ibuffer->UploadResources(gcl);
			}

			InputLayout il;
			il.SetVertexType(mesh->vertexType);

			si.geometry.CreateResources(std::move(vbuffer), std::move(ibuffer), std::move(il), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			si.meshRef = mesh;
			items.emplace_back(std::move(si));
		}

		Geometry * GetGeometryReference(Asset::Mesh * mesh) {
			for(auto & i : items) {
				if(i.meshRef == mesh) {
					return &i.geometry;
				}
			}
			return nullptr;
		}

	public:


		virtual void UploadResources(ID3D12GraphicsCommandList * copyCommandList) override {
			for(auto & i : items) {
				if(i.geometry.indexBuffer != nullptr) {
					i.geometry.indexBuffer->UploadResources(copyCommandList);
				}
				i.geometry.vertexBuffer->UploadResources(copyCommandList);
			}
		}

		virtual void ReleaseUploadResources() override {
			for(auto & i : items) {
				i.geometry.indexBuffer->ReleaseUploadResources();
				i.geometry.vertexBuffer->ReleaseUploadResources();
			}
		}

		virtual void CreateResources(ID3D12Device * dev) override {
			device = dev;
		}

		virtual void ReleaseResources() override {

		}

		Geometry * GetGeometry(Asset::Mesh * mesh) {
			if(!Exist(mesh)) {
				CreateGeometry(mesh);
			}
			return GetGeometryReference(mesh);
		}
	};

}
