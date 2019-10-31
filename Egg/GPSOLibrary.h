#pragma once

#include "ShaderCodeCollection.h"
#include "Geometry.h"

namespace Egg::Graphics::Internal {

	class GPSOLibrary {

		bool Exist(D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {

		}

		void CreateGPSO(D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {

		}

		ID3D12PipelineState * Find(D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc) {

		}

	public:
		ID3D12PipelineState * GetPipelineState(ID3D12RootSignature * rootSig, ShaderCodeCollection & scc, Geometry * geometry) {
			D3D12_GRAPHICS_PIPELINE_STATE_DESC gpso;
			ZeroMemory(&gpso, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			gpso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
			gpso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			gpso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			gpso.NumRenderTargets = 1;
			gpso.SampleMask = UINT_MAX;
			gpso.SampleDesc.Count = 1;
			gpso.SampleDesc.Quality = 0;
			gpso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			gpso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			gpso.InputLayout = geometry->GetInputLayout();

			if(Exist(gpso)) {
				CreateGPSO(gpso);
			}

			return Find(gpso);
		}
	};

}
