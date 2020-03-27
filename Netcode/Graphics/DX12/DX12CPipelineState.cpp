#include "DX12CPipelineState.h"
#include "DX12Helpers.h"
#include "DX12RootSignature.h"

namespace Netcode::Graphics::DX12 {

	void CPipelineStateDesc::FillNativeDesc(D3D12_COMPUTE_PIPELINE_STATE_DESC & cpsd)
	{
		cpsd.CachedPSO.pCachedBlob = nullptr;
		cpsd.CachedPSO.CachedBlobSizeInBytes = 0;
		cpsd.CS = GetNativeBytecode(CS);
		cpsd.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		cpsd.NodeMask = 0;
		cpsd.pRootSignature = static_cast<Netcode::Graphics::DX12::RootSignature *>(rootSignature->GetImplDetail())->GetNativeRootSignature();
	}

	bool CPipelineStateDesc::operator==(const CPipelineStateDesc & rhs) const
	{
		return rootSignature == rhs.rootSignature && CS == rhs.CS;
	}

	CPipelineState::CPipelineState(ID3D12Device * device, CPipelineStateDesc && cpsoDesc) : pso{ nullptr }, psoDesc { std::move(cpsoDesc) }
	{
		D3D12_COMPUTE_PIPELINE_STATE_DESC cpsd;
		psoDesc.FillNativeDesc(cpsd);

		device->CreateComputePipelineState(&cpsd, IID_PPV_ARGS(pso.GetAddressOf()));
	}

	ID3D12PipelineState * CPipelineState::GetNativePipelineState() const
	{
		return pso.Get();
	}

	const CPipelineStateDesc & CPipelineState::GetDesc() const
	{
		return psoDesc;
	}

	void * CPipelineState::GetImplDetail() const
	{
		return GetNativePipelineState();
	}

}
