#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12Includes.h"
#include "DX12Common.h"

namespace Netcode::Graphics::DX12 {

	class RootSignatureDesc {
		Unique<uint8_t[]> buffer;
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;

		void CreateRS_1_0(const D3D12_ROOT_SIGNATURE_DESC & rsDesc);

		void CreateRS_1_1(const D3D12_ROOT_SIGNATURE_DESC1 & rsDesc);

	public:
		RootSignatureDesc(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rsDesc);

		const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & GetNativeDesc() const;
	};

	class RootSignatureImpl : public RootSignature {
		com_ptr<ID3D12RootSignature> rootSignature;
		RootSignatureDesc desc;
	public:

		RootSignatureImpl(com_ptr<ID3D12RootSignature> rs, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rsDesc);

		const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & GetDesc() const;

		ID3D12RootSignature * GetNativeRootSignature() const;

		virtual void * GetImplDetail() const override;
	};

}
