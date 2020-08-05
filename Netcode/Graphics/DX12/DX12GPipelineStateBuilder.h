#pragma once

#include "DX12GPipelineState.h"

namespace Netcode::Graphics::DX12 {

	class GPipelineStateLibrary;

	class GPipelineStateBuilderImpl : public Netcode::GPipelineStateBuilder {
		GPipelineStateDesc desc;
		Ref<GPipelineStateLibrary> libraryRef;

	public:
		GPipelineStateBuilderImpl(Ref<GPipelineStateLibrary> libRef);

		virtual void SetRootSignature(Ref<RootSignature> rootSignature) override;

		virtual void SetDepthStencilState(const DepthStencilDesc & depthStencilState) override;

		virtual void SetRasterizerState(const RasterizerDesc & rasterizerState) override;

		virtual void SetBlendState(const BlendDesc & blendState) override;

		virtual void SetStreamOutput(Ref<StreamOutput> streamOutput) override;

		virtual void SetInputLayout(Ref<InputLayout> inputLayout) override;

		virtual void SetVertexShader(Ref<ShaderBytecode> shader) override;

		virtual void SetPixelShader(Ref<ShaderBytecode> shader) override;

		virtual void SetGeometryShader(Ref<ShaderBytecode> shader) override;

		virtual void SetHullShader(Ref<ShaderBytecode> shader) override;

		virtual void SetDomainShader(Ref<ShaderBytecode> shader) override;

		virtual void SetNumRenderTargets(uint8_t numRenderTargets) override;

		virtual void SetDepthStencilFormat(DXGI_FORMAT format) override;

		virtual void SetRenderTargetFormat(uint8_t renderTargetIdx, DXGI_FORMAT format) override;

		virtual void SetRenderTargetFormats(std::initializer_list<DXGI_FORMAT> formats) override;

		virtual void SetPrimitiveTopologyType(PrimitiveTopologyType topType) override;

		virtual Ref<PipelineState> Build() override;
	};

}
