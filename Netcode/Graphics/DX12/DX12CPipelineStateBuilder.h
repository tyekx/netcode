#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12CPipelineState.h"

namespace Netcode::Graphics::DX12 {

	class CPipelineStateLibrary;

	class CPipelineStateBuilderImpl : public Netcode::CPipelineStateBuilder {
		CPipelineStateDesc cDesc;
		Ref<CPipelineStateLibrary> cLib;
	public:
		CPipelineStateBuilderImpl(Ref<CPipelineStateLibrary> libRef);

		virtual void SetRootSignature(Ref<RootSignature> rootSig) override;
		virtual void SetComputeShader(Ref<ShaderBytecode> shader) override;
		virtual Ref<PipelineState> Build() override;
	};

}
