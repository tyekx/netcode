#include "DX12GPipelineStateBuilder.h"

namespace Netcode::Graphics::DX12 {

	GPipelineStateBuilder::GPipelineStateBuilder(Ref<DX12::GPipelineStateLibrary> libRef) : desc{}, libraryRef{ std::move( libRef ) } {
		
	}

	void GPipelineStateBuilder::SetRootSignature(Ref<Netcode::RootSignature> rootSignature) {
		desc.rootSignature = rootSignature;
	}

	void GPipelineStateBuilder::SetDepthStencilState(const DepthStencilDesc & depthStencilState) {
		desc.depthStencilState = depthStencilState;
	}

	void GPipelineStateBuilder::SetRasterizerState(const RasterizerDesc & rasterizerState) {
		desc.rasterizerState = rasterizerState;
	}

	void GPipelineStateBuilder::SetBlendState(const BlendDesc & blendState) {
		desc.blendState = blendState;
	}

	void GPipelineStateBuilder::SetStreamOutput(StreamOutputRef streamOutput) {
		desc.streamOutput = streamOutput;
	}

	void GPipelineStateBuilder::SetInputLayout(InputLayoutRef inputLayout) {
		desc.inputLayout = inputLayout;
	}

	void GPipelineStateBuilder::SetVertexShader(ShaderBytecodeRef shader) {
		desc.VS = shader;
	}

	void GPipelineStateBuilder::SetPixelShader(ShaderBytecodeRef shader) {
		desc.PS = shader;
	}

	void GPipelineStateBuilder::SetGeometryShader(ShaderBytecodeRef shader) {
		desc.GS = shader;
	}

	void GPipelineStateBuilder::SetHullShader(ShaderBytecodeRef shader) {
		desc.HS = shader;
	}

	void GPipelineStateBuilder::SetDomainShader(ShaderBytecodeRef shader) {
		desc.DS = shader;
	}

	void GPipelineStateBuilder::SetNumRenderTargets(uint8_t numRenderTargets)
	{
		desc.numRenderTargets = numRenderTargets;
	}

	void GPipelineStateBuilder::SetDepthStencilFormat(DXGI_FORMAT format)
	{
		desc.dsvFormat = format;
	}

	void GPipelineStateBuilder::SetRenderTargetFormat(uint8_t renderTargetIdx, DXGI_FORMAT format)
	{
		if(renderTargetIdx >= 0 && renderTargetIdx < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT) {
			desc.rtvFormats[renderTargetIdx] = format;
		}
	}

	void GPipelineStateBuilder::SetRenderTargetFormats(std::initializer_list<DXGI_FORMAT> formats)
	{
		uint8_t i = 0;

		SetNumRenderTargets(static_cast<uint8_t>(formats.size()));
		for(DXGI_FORMAT format : formats) {
			SetRenderTargetFormat(i++, format);
		}
	}

	void GPipelineStateBuilder::SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType topType)
	{
		desc.topologyType = topType;
	}

	PipelineStateRef GPipelineStateBuilder::Build() {
		return libraryRef->GetGraphicsPipelineState(std::move(desc));
	}

}
