#include "PipelineState.h"

namespace Egg {

	PipelineState::PipelineState() : rootSignature{ nullptr }, vertexShader{ nullptr }, geometryShader{ nullptr }, pixelShader{ nullptr }, blendState{}, rasterizerState{}, depthStencilState{} {
		blendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		rasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		depthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		depthStencilState.DepthEnable = FALSE;
		depthStencilState.StencilEnable = FALSE;
		topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		dsvFormat = DXGI_FORMAT_UNKNOWN;
	}

	com_ptr<ID3D12RootSignature> PipelineState::GetRootSignature() {
		return rootSignature;
	}

	void PipelineState::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveTopology) {
		topology = primitiveTopology;
	}

	void PipelineState::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC & dsd) {
		depthStencilState = dsd;
	}

	void PipelineState::SetRasterizerState(const D3D12_RASTERIZER_DESC & rsd) {
		rasterizerState = rsd;
	}

	void PipelineState::SetBlendState(const D3D12_BLEND_DESC & bsd) {
		blendState = bsd;
	}

	void PipelineState::SetRootSignature(com_ptr<ID3D12RootSignature> rs) {
		rootSignature = rs;
	}

	void PipelineState::SetVertexShader(com_ptr<ID3DBlob> vs) {
		vertexShader = vs;
	}

	void PipelineState::SetPixelShader(com_ptr<ID3DBlob> ps) {
		pixelShader = ps;
	}

	void PipelineState::SetGeometryShader(com_ptr<ID3DBlob> gs) {
		geometryShader = gs;
	}

	void PipelineState::SetDSVFormat(DXGI_FORMAT format) {
		dsvFormat = format;
	}

	void PipelineState::ApplyToDescriptor(D3D12_GRAPHICS_PIPELINE_STATE_DESC & psoDesc) {
		psoDesc.pRootSignature = rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		if(geometryShader != nullptr) {
			psoDesc.GS = CD3DX12_SHADER_BYTECODE(geometryShader.Get());
		}
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.BlendState = blendState;
		psoDesc.RasterizerState = rasterizerState;
		psoDesc.DepthStencilState = depthStencilState;
		psoDesc.DSVFormat = dsvFormat;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = topology;
	}

}
