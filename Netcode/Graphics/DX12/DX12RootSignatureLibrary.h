#pragma once

#include "DX12RootSignature.h"

namespace Netcode::Graphics::DX12 {

	class RootSignatureLibrary {
		com_ptr<ID3D12Device> device;
		std::vector<DX12RootSignatureRef> rootSigs;

		static bool IsCompatible(const D3D12_ROOT_SIGNATURE_DESC & lhs, const D3D12_ROOT_SIGNATURE_DESC & rhs);

		static bool IsCompatible(const D3D12_DESCRIPTOR_RANGE & lhs, const D3D12_DESCRIPTOR_RANGE & rhs);

		static bool IsCompatible(const D3D12_ROOT_PARAMETER & lhs, const D3D12_ROOT_PARAMETER & rhs);

		static bool IsCompatible(const D3D12_DESCRIPTOR_RANGE1 & lhs, const D3D12_DESCRIPTOR_RANGE1 & rhs);

		static bool IsCompatible(const D3D12_ROOT_PARAMETER1 & lhs, const D3D12_ROOT_PARAMETER1 & rhs);

		static bool IsCompatible(const D3D12_STATIC_SAMPLER_DESC & lhs, const D3D12_STATIC_SAMPLER_DESC & rhs);

		static bool IsCompatible(const D3D12_ROOT_SIGNATURE_DESC1 & lhs, const D3D12_ROOT_SIGNATURE_DESC1 & rhs);

		// does LHS provide a compatible interface for RHS
		static bool IsCompatible(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & lhs, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rhs);
	public:
		void SetDevice(com_ptr<ID3D12Device> dev);

		RootSignatureRef GetRootSignature(ShaderBytecodeRef blobWithRootSig);

		RootSignatureRef GetRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & desc);
	};

	using DX12RootSignatureLibrary = Netcode::Graphics::DX12::RootSignatureLibrary;
	using DX12RootSignatureLibraryRef = std::shared_ptr<DX12RootSignatureLibrary>;
}
