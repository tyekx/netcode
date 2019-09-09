#pragma once

#include "Common.h"

namespace Egg {

	class PsoManager {
		com_ptr<ID3D12Device> device;
		std::vector<D3D12_GRAPHICS_PIPELINE_STATE_DESC> gpsoDescs;
		std::vector< com_ptr<ID3D12PipelineState> > gpsos;
		

		bool InputLayoutIsCompatible(const D3D12_INPUT_LAYOUT_DESC & lhs, const D3D12_INPUT_LAYOUT_DESC & rhs);

		bool RenderTargetsAreCompatible(const DXGI_FORMAT * lhsFormats, const DXGI_FORMAT * rhsFormats, unsigned int numRenderTargets);

		bool DepthStencilOpEqual(const D3D12_DEPTH_STENCILOP_DESC & lhs, const D3D12_DEPTH_STENCILOP_DESC & rhs);

		bool DepthStencilEqual(const D3D12_DEPTH_STENCIL_DESC & lhs, const D3D12_DEPTH_STENCIL_DESC & rhs);

		bool RasterizerStateEqual(const D3D12_RASTERIZER_DESC & lhs, const D3D12_RASTERIZER_DESC & rhs);
		
		bool ByteCodesEqual(const D3D12_SHADER_BYTECODE & lhs, const D3D12_SHADER_BYTECODE & rhs);

		bool Equal(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & lhs, const D3D12_GRAPHICS_PIPELINE_STATE_DESC & rhs);

		int Exists(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & sDesc);

	public:
		PsoManager(com_ptr<ID3D12Device> dev);

		com_ptr<ID3D12PipelineState> Get(const D3D12_GRAPHICS_PIPELINE_STATE_DESC & gpsoDesc);

		com_ptr<ID3D12PipelineState> Get(ID3D12RootSignature * rootSignature, const D3D12_INPUT_LAYOUT_DESC & inputLayout, const D3D12_SHADER_BYTECODE & vertexShader, const D3D12_SHADER_BYTECODE & pixelShader);
	};

}

