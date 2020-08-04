#pragma once

#include "../../HandleTypes.h"
#include "../ResourceEnums.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	struct GPipelineStateDesc {
		Ref<RootSignature> rootSignature;
		DepthStencilDesc depthStencilState;
		BlendDesc blendState;
		Ref<StreamOutput> streamOutput;
		Ref<InputLayout> inputLayout;
		RasterizerDesc rasterizerState;
		Ref<ShaderBytecode> VS;
		Ref<ShaderBytecode> PS;
		Ref<ShaderBytecode> GS;
		Ref<ShaderBytecode> HS;
		Ref<ShaderBytecode> DS;
		DXGI_FORMAT rtvFormats[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		DXGI_FORMAT dsvFormat;
		PrimitiveTopologyType topologyType;
		uint32_t numRenderTargets;

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
