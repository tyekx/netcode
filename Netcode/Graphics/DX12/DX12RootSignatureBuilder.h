#pragma once

#include "../../HandleTypes.h"
#include "DX12RootSignature.h"
#include "DX12RootSignatureLibrary.h"

namespace Netcode::Graphics::DX12 {

	class RootSignatureBuilder : public Netcode::RootSignatureBuilder {
		Ref<DX12::RootSignatureLibrary> rootSigLibrary;
	public:
		RootSignatureBuilder(Ref<DX12::RootSignatureLibrary> libRef);

		virtual Ref<Netcode::RootSignature> Build() override;

		virtual Ref<Netcode::RootSignature> BuildFromShader(Ref<Netcode::ShaderBytecode> rootSigContainingBytecode) override;

		virtual Ref<Netcode::RootSignature> BuildEmpty() override;
	};

	using DX12RootSignatureBuilder = Netcode::Graphics::DX12::RootSignatureBuilder;
	using DX12RootSignatureBuilderRef = std::shared_ptr<DX12RootSignatureBuilder>;

}
