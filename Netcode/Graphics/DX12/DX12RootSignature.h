#pragma once

#include "../../HandleTypes.h"
#include "DX12Common.h"

namespace Egg::Graphics::DX12 {

	class RootSignatureDesc {
		std::unique_ptr<uint8_t[]> buffer;
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;

		void CreateRS_1_0(const D3D12_ROOT_SIGNATURE_DESC & rsDesc);

		void CreateRS_1_1(const D3D12_ROOT_SIGNATURE_DESC1 & rsDesc);

	public:
		RootSignatureDesc(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rsDesc);

		const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & GetNativeDesc() const;
	};

	class RootSignature : public Egg::RootSignature {
		com_ptr<ID3D12RootSignature> rootSignature;
		RootSignatureDesc desc;
	public:

		RootSignature(com_ptr<ID3D12RootSignature> rs, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rsDesc);

		const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & GetDesc() const;

		ID3D12RootSignature * GetNativeRootSignature() const;

		virtual void * GetImplDetail() const override;
	};

	using DX12RootSignature = Egg::Graphics::DX12::RootSignature;
	using DX12RootSignatureRef = std::shared_ptr<DX12RootSignature>;

}
