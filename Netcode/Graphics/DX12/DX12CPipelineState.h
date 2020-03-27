#pragma once

#include "../../HandleTypes.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	struct CPipelineStateDesc {
		RootSignatureRef rootSignature;
		ShaderBytecodeRef CS;

		void FillNativeDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC & cpsd);

		bool operator==(const CPipelineStateDesc & rhs) const;
	};

	class CPipelineState : public Netcode::PipelineState {
		com_ptr<ID3D12PipelineState> pso;
		CPipelineStateDesc psoDesc;

	public:
		CPipelineState(ID3D12Device * device, CPipelineStateDesc && cpsoDesc);

		ID3D12PipelineState * GetNativePipelineState() const;

		const CPipelineStateDesc & GetDesc() const;

		virtual void * GetImplDetail() const override;
	};

	using DX12CPipelineState = Netcode::Graphics::DX12::CPipelineState;
	using DX12CPipelineStateRef = std::shared_ptr<DX12CPipelineState>;

}
