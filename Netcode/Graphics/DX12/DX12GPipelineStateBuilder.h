#pragma once

#include "DX12GPipelineState.h"
#include "DX12GPipelineStateLibrary.h"

namespace Netcode::Graphics::DX12 {

	class GPipelineStateBuilder : public Netcode::GPipelineStateBuilder {
		GPipelineStateDesc desc;
		DX12GPipelineStateLibraryRef libraryRef;

	public:
		GPipelineStateBuilder(DX12GPipelineStateLibraryRef libRef);

		virtual void SetRootSignature(RootSignatureRef rootSignature) override;

		virtual void SetDepthStencilState(const DepthStencilDesc & depthStencilState) override;

		virtual void SetRasterizerState(const RasterizerDesc & rasterizerState) override;

		virtual void SetBlendState(const BlendDesc & blendState) override;

		virtual void SetStreamOutput(StreamOutputRef streamOutput) override;

		virtual void SetInputLayout(InputLayoutRef inputLayout) override;

		virtual void SetVertexShader(ShaderBytecodeRef shader) override;

		virtual void SetPixelShader(ShaderBytecodeRef shader) override;

		virtual void SetGeometryShader(ShaderBytecodeRef shader) override;

		virtual void SetHullShader(ShaderBytecodeRef shader) override;

		virtual void SetDomainShader(ShaderBytecodeRef shader) override;

		virtual void SetNumRenderTargets(uint8_t numRenderTargets) override;

		virtual void SetDepthStencilFormat(DXGI_FORMAT format) override;

		virtual void SetRenderTargetFormat(uint8_t renderTargetIdx, DXGI_FORMAT format) override;

		virtual void SetRenderTargetFormats(std::initializer_list<DXGI_FORMAT> formats) override;

		virtual void SetPrimitiveTopologyType(Netcode::Graphics::PrimitiveTopologyType topType) override;

		virtual PipelineStateRef Build() override;
	};

	using DX12GPipelineStateBuilder = Netcode::Graphics::DX12::GPipelineStateBuilder;
	using DX12GPipelineStateBuilderRef = std::shared_ptr<Netcode::Graphics::DX12::GPipelineStateBuilder>;
}
