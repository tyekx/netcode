#include "../../Common.h"
#include "DX12RootSignatureBuilder.h"

namespace  Egg::Graphics::DX12 {

	RootSignatureBuilder::RootSignatureBuilder(DX12RootSignatureLibraryRef libRef) :rootSigLibrary{ std::move(libRef) } { }

	RootSignatureRef RootSignatureBuilder::Build() {
		//@TODO: support manual creation of root signature
		ASSERT(false, "Building from scratch is not supported yet");

		return nullptr;
	}
	RootSignatureRef RootSignatureBuilder::BuildFromShader(ShaderBytecodeRef rootSigContainingBytecode) {
		return rootSigLibrary->GetRootSignature(rootSigContainingBytecode);
	}

}