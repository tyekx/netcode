#include "DX12CPipelineStateBuilder.h"

namespace Netcode::Graphics::DX12 {

	CPipelineStateBuilder::CPipelineStateBuilder(DX12CPipelineStateLibraryRef libRef) : cDesc{}, cLib{ std::move(libRef) }
	{
	}

	void CPipelineStateBuilder::SetRootSignature(RootSignatureRef rootSig)
	{
		cDesc.rootSignature = std::move(rootSig);
	}

	void CPipelineStateBuilder::SetComputeShader(ShaderBytecodeRef shader)
	{
		cDesc.CS = std::move(shader);
	}

	PipelineStateRef CPipelineStateBuilder::Build()
	{
		return cLib->GetComputePipelineState(std::move(cDesc));
	}

}

