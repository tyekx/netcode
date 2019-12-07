#pragma once

#include "DX12Resource.h"
#include "InputLayout.h"
#include "DX12RenderItem.h"

namespace Egg::Graphics {


	class Geometry {
	public:
		using vbuffer_type = std::unique_ptr<DX12::Resource::AVBuffer>;
		using ibuffer_type = std::unique_ptr<DX12::Resource::AIBuffer>;

		vbuffer_type vertexBuffer;
		ibuffer_type indexBuffer;
		InputLayout inputLayout;

		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType;
		D3D12_PRIMITIVE_TOPOLOGY topology;
		D3D12_FILL_MODE mode;

		void CreateResources(vbuffer_type && vbufferData, ibuffer_type && ibufferData, InputLayout && layout,
							 D3D12_PRIMITIVE_TOPOLOGY_TYPE topType, D3D12_PRIMITIVE_TOPOLOGY top, D3D12_FILL_MODE fillMode = D3D12_FILL_MODE_SOLID) {
			vertexBuffer = std::move(vbufferData);
			indexBuffer = std::move(ibufferData);
			inputLayout = std::move(layout);
			topologyType = topType;
			topology = top;
			mode = fillMode;
		}

		void FillRenderItem(DX12::RenderItem * renderItem) {
			renderItem->lodLevelsLength = vertexBuffer->GetLODCount();
			for(UINT i = 0; i < renderItem->lodLevelsLength; ++i) {
				const auto & lod = vertexBuffer->GetLOD(i);
				renderItem->lodLevels[i].vertexBufferView = lod.vertexBufferView;
				renderItem->lodLevels[i].vertexCount = lod.verticesCount;

				if(indexBuffer) {
					const auto & iLod = indexBuffer->GetLOD(i);
					renderItem->lodLevels[i].indexBufferView = iLod.indexBufferView;
					renderItem->lodLevels[i].indexCount = iLod.indexCount;
				}
			}

			renderItem->primitiveTopology = topology;
		}

		inline const D3D12_INPUT_LAYOUT_DESC& GetInputLayout() {
			return inputLayout.GetInputLayout();
		}
	};

}
