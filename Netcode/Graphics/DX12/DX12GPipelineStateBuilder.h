#pragma once

#include "DX12GPipelineState.h"
#include "DX12GPipelineStateLibrary.h"

namespace Netcode::Graphics::DX12 {

	class GPipelineStateBuilder : public Netcode::GPipelineStateBuilder {
		GPipelineStateDesc desc;
		Ref<DX12::GPipelineStateLibrary> libraryRef;

	public:
		GPipelineStateBuilder(Ref<DX12::GPipelineStateLibrary> libRef);

		virtual void SetRootSignature(Ref<Netcode::RootSignature> rootSignature) override;

		virtual void SetDepthStencilState(const DepthStencilDesc & depthStencilState) override;

		virtual void SetRasterizerState(const RasterizerDesc & rasterizerState) override;

		virtual void SetBlendState(const BlendDesc & blendState) override;

		virtual void SetStreamOutput(Ref<Netcode::StreamOutput> streamOutput) override;

		virtual void SetInputLayout(Ref<Netcode::InputLayout> inputLayout) override;

		virtual void SetVertexShader(Ref<Netcode::ShaderBytecode> shader) override;

		virtual void SetPixelShader(Ref<Netcode::ShaderBytecode> shader) override;

		virtual void SetGeometryShader(Ref<Netcode::ShaderBytecode> shader) override;

		virtual void SetHullShader(Ref<Netcode::ShaderBytecode> shader) override;

		virtual void SetDomainShader(Ref<Netcode::ShaderBytecode> shader) override;

		virtual void SetNumRenderTargets(uint8_t numRenderTargets) override;

		virtual void SetDepthStencilFormat(DXGI_FORMAT format) override;

		virtual void SetRenderTargetFormat(uint8_t renderTargetIdx, DXGI_FORMAT format) override;

		virtual void SetRenderTargetFormats(std::initializer_list<DXGI_FORMAT> formats) override;

		virtual void SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType topType) override;

		virtual Ref<Netcode::PipelineState> Build() override;
	};

}
