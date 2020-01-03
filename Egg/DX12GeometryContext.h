#pragma once

#include "HandleTypes.h"
#include "DX12Common.h"
#include "GraphicsContexts.h"

namespace Egg::Graphics::DX12 {

	class GeometryContext : public Egg::Graphics::IGeometryContext {

		struct Item {
			std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
			D3D12_INPUT_LAYOUT_DESC cachedLayoutDesc;
			bool isFinalized;
		};

		std::vector<Item> storage;

		Item & Get(HGEOMETRY geometry) {
			ASSERT(geometry >= 0 && geometry < storage.size(), "Geometry was not found");
			return storage.at(geometry);
		}

	public:

		const D3D12_INPUT_LAYOUT_DESC & GetInputLayout(HGEOMETRY geometry) {
			Item & i = Get(geometry);

			if(!i.isFinalized) {
				i.cachedLayoutDesc.NumElements = static_cast<UINT>(i.inputLayout.size());
				i.cachedLayoutDesc.pInputElementDescs = &(i.inputLayout.at(0));

				ASSERT(i.cachedLayoutDesc.NumElements != 0, "While finalizing geometry: no input elements were submitted");

				i.isFinalized = true;
			}

			return i.cachedLayoutDesc;
		}

		HGEOMETRY CreateGeometry() {
			HGEOMETRY idx = static_cast<HGEOMETRY>(storage.size());
			Item item;
			item.isFinalized = false;
			storage.emplace_back(std::move(item));
			return idx;
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, DXGI_FORMAT format) override {
			AddInputElement(geometry, name, semanticIndex, format, D3D12_APPEND_ALIGNED_ELEMENT);
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, DXGI_FORMAT format, unsigned int byteOffset) override {
			AddInputElement(geometry, name, 0, format, byteOffset);
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, DXGI_FORMAT format) override {
			AddInputElement(geometry, name, 0, format, D3D12_APPEND_ALIGNED_ELEMENT);
		}

		virtual void AddInputElement(HGEOMETRY geometry, const char * name, unsigned int semanticIndex, DXGI_FORMAT format, unsigned int byteOffset) override {
			Item & i = Get(geometry);

			ASSERT(!i.isFinalized, "Can not make modifications on a finalized geometry, an input element was added after creating a reference to it.");
			i.inputLayout.push_back({ name, semanticIndex, format, 0, byteOffset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		}

	};

}
