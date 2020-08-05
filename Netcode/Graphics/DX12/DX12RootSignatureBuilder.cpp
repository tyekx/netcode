#include "DX12RootSignatureBuilder.h"
#include <Netcode/Common.h>
#include "DX12RootSignatureLibrary.h"
#include "DX12RootSignature.h"
#include "DX12Includes.h"

namespace  Netcode::Graphics::DX12 {

	RootSignatureBuilderImpl::RootSignatureBuilderImpl(Ref<DX12::RootSignatureLibrary> libRef) :rootSigLibrary{ std::move(libRef) } { }

	Ref<RootSignature> RootSignatureBuilderImpl::Build() {
		ASSERT(false, "Building from scratch is not supported yet");

		return nullptr;
	}
	Ref<RootSignature> RootSignatureBuilderImpl::BuildFromShader(Ref<ShaderBytecode> rootSigContainingBytecode) {
		return rootSigLibrary->GetRootSignature(rootSigContainingBytecode);
	}

	Ref<RootSignature> RootSignatureBuilderImpl::BuildEmpty()
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