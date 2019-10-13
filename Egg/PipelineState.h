#pragma once

#include "Common.h"

namespace Egg {

		GG_CLASS(PipelineState)

			com_ptr<ID3D12RootSignature> rootSignature;
			com_ptr<ID3DBlob> vertexShader;
			com_ptr<ID3DBlob> geometryShader;
			com_ptr<ID3DBlob> pixelShader;
			D3D12_BLEND_DESC blendState;
			D3D12_RASTERIZER_DESC rasterizerState;
			D3D12_DEPTH_STENCIL_DESC depthStencilState;
			DXGI_FORMAT dsvFormat;
		public:

			PipelineState();

			com_ptr<ID3D12RootSignature> GetRootSignature();

			void SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC & dsd);

			void SetRasterizerState(const D3D12_RASTERIZER_DESC & rsd);

			void SetBlendState(const D3D12_BLEND_DESC & bsd);

			void SetRootSignature(com_ptr<ID3D12RootSignature> rs);

			void SetVertexShader(com_ptr<ID3DBlob> vs);

			void SetPixelShader(com_ptr<ID3DBlob> ps);

			void SetGeometryShader(com_ptr<ID3DBlob> gs);

			void SetDSVFormat(DXGI_FORMAT format);

			void ApplyToDescriptor(D3D12_GRAPHICS_PIPELINE_STATE_DESC & psoDesc);


		GG_ENDCLASS

}
