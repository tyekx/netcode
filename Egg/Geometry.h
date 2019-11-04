#pragma once

#include "Resource.h"
#include "Mesh.h"
#include "InputLayout.h"

namespace Egg::Graphics {


	class Geometry {
	public:
		using vbuffer_type = std::unique_ptr<Resource::AVBuffer>;
		using ibuffer_type = std::unique_ptr<Resource::AIBuffer>;

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

		inline const D3D12_INPUT_LAYOUT_DESC& GetInputLayout() {
			return inputLayout.GetInputLayout();
		}

		Mesh GetMesh() {
			Mesh m;

			m.indexCount = 0;
			if(indexBuffer) {
				m.ibv = indexBuffer->GetView();
				UINT stride = (m.ibv.Format == DXGI_FORMAT_R32_UINT) ? 4 : 2;
				m.indexCount = m.ibv.SizeInBytes / stride;
			}
			m.vbv = vertexBuffer->GetView();

			m.vertexCount = m.vbv.SizeInBytes / m.vbv.StrideInBytes;

			m.topology = topology;
			m.perMeshCb = nullptr;

			return m;
		}
	};

}
