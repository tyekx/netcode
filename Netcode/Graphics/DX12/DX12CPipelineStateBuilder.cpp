#include "DX12CPipelineStateBuilder.h"
#include "DX12CPipelineStateLibrary.h"

namespace Netcode::Graphics::DX12 {

	CPipelineStateBuilderImpl::CPipelineStateBuilderImpl(Ref<CPipelineStateLibrary> libRef) : cDesc{}, cLib{ std::move(libRef) }
	{
	}

	void CPipelineStateBuilderImpl::SetRootSignature(Ref<RootSignature>  rootSig)
	{
		cDesc.rootSignature = std::move(rootSig);
	}

	void CPipelineStateBuilderImpl::SetComputeShader(Ref<ShaderBytecode> shader)
	{
		cDesc.CS = std::move(shader);
	}

	Ref<PipelineState> CPipelineStateBuilderImpl::Build()
	{
		return cLib->GetComputePipelineState(std::move(cDesc));
	}

}

