#include "../../Common.h"
#include "DX12RootSignatureBuilder.h"

namespace  Netcode::Graphics::DX12 {

	RootSignatureBuilder::RootSignatureBuilder(DX12RootSignatureLibraryRef libRef) :rootSigLibrary{ std::move(libRef) } { }

	RootSignatureRef RootSignatureBuilder::Build() {
		//@TODO: support manual creation of root signature
		ASSERT(false, "Building from scratch is not supported yet");

		return nullptr;
	}
	RootSignatureRef RootSignatureBuilder::BuildFromShader(ShaderBytecodeRef rootSigContainingBytecode) {
		return rootSigLibrary->GetRootSignature(rootSigContainingBytecode);
	}

	RootSignatureRef RootSignatureBuilder::BuildEmpty()
	{
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC vrsd;
		vrsd.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		vrsd.Desc_1_0.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		vrsd.Desc_1_0.NumParameters = 0;
		vrsd.Desc_1_0.pParameters = nullptr;
		vrsd.Desc_1_0.NumStaticSamplers = 0;
		vrsd.Desc_1_0.pStaticSamplers = nullptr;
		return rootSigLibrary->GetRootSignature(vrsd);
	}

}