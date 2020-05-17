#pragma once

#include "../../HandleTypes.h"
#include "DX12RootSignature.h"
#include "DX12RootSignatureLibrary.h"

namespace Netcode::Graphics::DX12 {

	class RootSignatureBuilder : public Netcode::RootSignatureBuilder {
		DX12RootSignatureLibraryRef rootSigLibrary;
	public:
		RootSignatureBuilder(DX12RootSignatureLibraryRef libRef);

		virtual RootSignatureRef Build() override;

		virtual RootSignatureRef BuildFromShader(ShaderBytecodeRef rootSigContainingBytecode) override;

		virtual RootSignatureRef BuildEmpty() override;
	};

	using DX12RootSignatureBuilder = Netcode::Graphics::DX12::RootSignatureBuilder;
	using DX12RootSignatureBuilderRef = std::shared_ptr<DX12RootSignatureBuilder>;

}
