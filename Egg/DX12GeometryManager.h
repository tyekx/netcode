#pragma once

#include "Common.h"
#include "HandleTypes.h"
#include "DX12PlacedVBuffer.h"
#include "DX12PlacedIBuffer.h"
#include "DX12RenderItem.h"

namespace Egg::Graphics::DX12 {

	class GeometryManager {

		struct Item {
			EGeometryType type;
			std::unique_ptr<Resource::PlacedVBuffer> vbuffer;
			std::unique_ptr<Resource::PlacedIBuffer> ibuffer;

			std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
			D3D12_INPUT_LAYOUT_DESC cachedLayoutDesc;
			bool isFinalized;
		};

		std::vector<Item> storage;
		std::vector<Resource::IUploadResource *> uploadPile;
		ID3D12Device * device;

		Item & Get(HGEOMETRY geometry) {
			ASSERT(geometry >= 0 && geometry < storage.size(), "Geometry was not found");
			return storage.at(geometry);
		}

	public:
		void UploadResources(Resource::IResourceUploader * uploader) {
			for(Resource::IUploadResource * r : uploadPile) {
				r->UploadResources(uploader);
			}
			uploadPile.clear();
		}

		void CreateResources(ID3D12Device * dev) {
			device = dev;
		}

		const D3D12_INPUT_LAYOUT_DESC & GetInputLayout(HGEOMETRY geometry) {
			Item & i = Get(geometry);

			if(!i.isFinalized) {
				i.vbuffer->CreateResources(device);
				uploadPile.push_back(i.vbuffer.get());
				if(i.type == EGeometryType::INDEXED) {
					i.ibuffer->CreateResources(device);
					uploadPile.push_back(i.ibuffer.get());
				}
				i.cachedLayoutDesc.NumElements = static_cast<UINT>(i.inputLayout.size());
				i.cachedLayoutDesc.pInputElementDescs = &(i.inputLayout.at(0));

				ASSERT(i.cachedLayoutDesc.NumElements != 0, "While finalizing geometry: no input elements were submitted");

				i.isFinalized = true;
			}

			return i.cachedLayoutDesc;
		}

		HGEOMETRY CreateGeometry(EGeometryType type) {
			HGEOMETRY idx = static_cast<HGEOMETRY>(storage.size());

			Item item;
			item.type = type;
			item.vbuffer = std::make_unique<Resource::PlacedVBuffer>();
			item.isFinalized = false;

			if(item.type == EGeometryType::INDEXED) {
				item.ibuffer = std::make_unique<Resource::PlacedIBuffer>();
			}

			storage.emplace_back(std::move(item));
			return idx;
		}

		void AddVertexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, unsigned int strideInBytes)   {
			Item & i = Get(geometry);
			ASSERT(!i.isFinalized, "Can not make modifications on a finalized geometry, a LOD level was added after creating a reference to it.");
			i.vbuffer->AddLODLevel(device, ptr, sizeInBytes, strideInBytes);
		}

		void AddIndexBufferLOD(HGEOMETRY geometry, void * ptr, unsigned int sizeInBytes, DXGI_FORMAT format)  {
			Item & i = Get(geometry);
			ASSERT(!i.isFinalized, "Can not make modifications on a finalized geometry, a LOD level was added after creating a reference to it.");
			ASSERT(i.type == EGeometryType::INDEXED, "Can't add index buffer level of detail to non indexed type geometry");
			if(i.ibuffer != nullptr) {
				i.ibuffer->AddLODLevel(device, ptr, sizeInBytes, format);
			}
		}
		
		void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) {
			Item & i = Get(geometry);

			ASSERT(!i.isFinalized, "Can not make modifications on a finalized geometry, an input element was added after creating a reference to it.");
			i.inputLayout.push_back({ name, semanticIndex, format, 0, byteOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}

		void SetRenderItemGeometry(RenderItem * renderItem, HGEOMETRY geometry) {
			Item & item = Get(geometry);

			renderItem->lodLevelsLength = item.vbuffer->GetLODCount();
			//@TODO: hardcoded topology
			renderItem->primitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			
			for(UINT i = 0; i < item.vbuffer->GetLODCount(); ++i) {
				auto & lod = item.vbuffer->GetLOD(i);
				renderItem->lodLevels[i].vertexBufferView = lod.vertexBufferView;
				renderItem->lodLevels[i].vertexCount = lod.verticesCount;
			}

			if(item.type == EGeometryType::INDEXED) {
				ASSERT(item.ibuffer != nullptr, "item is indexed but no index buffer is present");
				ASSERT(item.vbuffer->GetLODCount() == item.ibuffer->GetLODCount(), "Mismatching LOD level counts");
				for(UINT i = 0; i < item.ibuffer->GetLODCount(); ++i) {
					auto & lod = item.ibuffer->GetLOD(i);
					renderItem->lodLevels[i].indexBufferView = lod.indexBufferView;
					renderItem->lodLevels[i].indexCount = lod.indexCount;
				}
			}

		}
	};

}
