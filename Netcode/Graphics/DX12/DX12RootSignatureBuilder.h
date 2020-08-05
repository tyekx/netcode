#pragma once

#include <Netcode/HandleTypes.h>
#include <memory>

namespace Netcode::Graphics::DX12 {

	class RootSignatureLibrary;

	class RootSignatureBuilderImpl : public RootSignatureBuilder {
		Ref<RootSignatureLibrary> rootSigLibrary;
	public:
		RootSignatureBuilderImpl(Ref<RootSignatureLibrary> libRef);

		virtual Ref<RootSignature> Build() override;

		virtual Ref<RootSignature> BuildFromShader(Ref<ShaderBytecode> rootSigContainingBytecode) override;

		virtual Ref<RootSignature> BuildEmpty() override;
	};

}
