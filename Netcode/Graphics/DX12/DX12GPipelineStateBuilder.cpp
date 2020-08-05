#include "DX12GPipelineStateBuilder.h"
#include "DX12GPipelineStateLibrary.h"
#include "DX12RootSignature.h"
#include "DX12InputLayout.h"
#include "DX12StreamOutput.h"

namespace Netcode::Graphics::DX12 {

	GPipelineStateBuilderImpl::GPipelineStateBuilderImpl(Ref<GPipelineStateLibrary> libRef) : desc{}, libraryRef{ std::move( libRef ) } {
		
	}

	void GPipelineStateBuilderImpl::SetRootSignature(Ref<RootSignature> rootSignature) {
		desc.rootSignature = std::dynamic_pointer_cast<RootSignatureImpl>(rootSignature);
	}

	void GPipelineStateBuilderImpl::SetDepthStencilState(const DepthStencilDesc & depthStencilState) {
		desc.depthStencilState = depthStencilState;
	}

	void GPipelineStateBuilderImpl::SetRasterizerState(const RasterizerDesc & rasterizerState) {
		desc.rasterizerState = rasterizerState;
	}

	void GPipelineStateBuilderImpl::SetBlendState(const BlendDesc & blendState) {
		desc.blendState = blendState;
	}

	void GPipelineStateBuilderImpl::SetStreamOutput(Ref<StreamOutput> streamOutput) {
		desc.streamOutput = std::dynamic_pointer_cast<StreamOutputImpl>(streamOutput);
	}

	void GPipelineStateBuilderImpl::SetInputLayout(Ref<InputLayout> inputLayout) {
		desc.inputLayout = std::dynamic_pointer_cast<InputLayoutImpl>(inputLayout);
	}

	void GPipelineStateBuilderImpl::SetVertexShader(Ref<ShaderBytecode> shader) {
		desc.VS = std::move(shader);
	}

	void GPipelineStateBuilderImpl::SetPixelShader(Ref<ShaderBytecode> shader) {
		desc.PS = std::move(shader);
	}

	void GPipelineStateBuilderImpl::SetGeometryShader(Ref<ShaderBytecode> shader) {
		desc.GS = std::move(shader);
	}

	void GPipelineStateBuilderImpl::SetHullShader(Ref<ShaderBytecode> shader) {
		desc.HS = std::move(shader);
	}

	void GPipelineStateBuilderImpl::SetDomainShader(Ref<ShaderBytecode> shader) {
		desc.DS = std::move(shader);
	}

	void GPipelineStateBuilderImpl::SetNumRenderTargets(uint8_t numRenderTargets)
	{
		desc.numRenderTargets = numRenderTargets;
	}

	void GPipelineStateBuilderImpl::SetDepthStencilFormat(DXGI_FORMAT format)
	{
		desc.dsvFormat = format;
	}

	void GPipelineStateBuilderImpl::SetRenderTargetFormat(uint8_t renderTargetIdx, DXGI_FORMAT format)
	{
		if(renderTargetIdx >= 0 && renderTargetIdx < 8) {
			desc.rtvFormats[renderTargetIdx] = format;
		}
	}

	void GPipelineStateBuilderImpl::SetRenderTargetFormats(std::initializer_list<DXGI_FORMAT> formats)
	{
		uint8_t i = 0;

		SetNumRenderTargets(static_cast<uint8_t>(formats.size()));
		for(DXGI_FORMAT format : formats) {
			SetRenderTargetFormat(i++, format);
		}
	}

	void GPipelineStateBuilderImpl::SetPrimitiveTopologyType(PrimitiveTopologyType topType)
	{
		desc.topologyType = topType;
	}

	Ref<Netcode::PipelineState> GPipelineStateBuilderImpl::Build() {
		return libraryRef->GetGraphicsPipelineState(std::move(desc));
	}

}
