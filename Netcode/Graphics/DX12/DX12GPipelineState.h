#pragma once

#include "../../HandleTypes.h"
#include "../ResourceEnums.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	struct GPipelineStateDesc {
		RootSignatureRef rootSignature;
		DepthStencilDesc depthStencilState;
		BlendDesc blendState;
		StreamOutputRef streamOutput;
		InputLayoutRef inputLayout;
		RasterizerDesc rasterizerState;
		ShaderBytecodeRef VS;
		ShaderBytecodeRef PS;
		ShaderBytecodeRef GS;
		ShaderBytecodeRef HS;
		ShaderBytecodeRef DS;
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

	using DX12GPipelineState = Netcode::Graphics::DX12::GPipelineState;
	using DX12GPipelineStateRef = std::shared_ptr<DX12GPipelineState>;

}
