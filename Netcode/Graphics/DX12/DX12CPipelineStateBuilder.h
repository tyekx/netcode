#pragma once

#include "../../HandleTypes.h"
#include "DX12CPipelineState.h"
#include "DX12CPipelineStateLibrary.h"

namespace Netcode::Graphics::DX12 {

	class CPipelineStateBuilder : public Netcode::CPipelineStateBuilder {
		CPipelineStateDesc cDesc;
		DX12CPipelineStateLibraryRef cLib;
	public:
		CPipelineStateBuilder(DX12CPipelineStateLibraryRef libRef);

		virtual void SetRootSignature(RootSignatureRef rootSig) override;
		virtual void SetComputeShader(ShaderBytecodeRef shader) override;
		virtual PipelineStateRef Build() override;
	};

	using DX12CPipelineStateBuilder = Netcode::Graphics::DX12::CPipelineStateBuilder;
	using DX12CPipelineStateBuilderRef = std::shared_ptr<DX12CPipelineStateBuilder>;

}
