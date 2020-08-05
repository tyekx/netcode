#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12Decl.h"
#include "DX12Common.h"
#include "DX12Platform.h"
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	class StreamOutputImpl;
	class InputLayoutImpl;
	class RootSignatureImpl;

	struct GPipelineStateDesc {
		DepthStencilDesc depthStencilState;
		BlendDesc blendState;
		RasterizerDesc rasterizerState;
		DXGI_FORMAT rtvFormats[Platform::SimultaneousRenderTargetCount];
		DXGI_FORMAT dsvFormat;
		PrimitiveTopologyType topologyType;
		uint32_t numRenderTargets;
		Ref<RootSignatureImpl> rootSignature;
		Ref<StreamOutputImpl> streamOutput;
		Ref<InputLayoutImpl> inputLayout;
		Ref<ShaderBytecode> VS;
		Ref<ShaderBytecode> PS;
		Ref<ShaderBytecode> GS;
		Ref<ShaderBytecode> HS;
		Ref<ShaderBytecode> DS;

		void FillNativeDesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC & psd);

		bool operator==(const GPipelineStateDesc & rhs) const;
	};

	class GPipelineState : public Netcode::PipelineState {
		com_ptr<ID3D12PipelineState> pso;
		GPipelineStateDesc psoDesc;

	public:
		GPipelineState(ID3D12Device * device, GPipelineStateDesc && gpsoDesc);

		inline ID3D12PipelineState * GetNativePipelineState() const;

		const GPipelineStateDesc & GetDesc() const;

		virtual void * GetImplDetail() const override;

	};

}
