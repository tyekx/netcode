#include "DX12RootSignatureLibrary.h"
#include <Netcode/HandleTypes.h>
#include "DX12RootSignature.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	bool RootSignatureLibrary::IsCompatible(const D3D12_ROOT_SIGNATURE_DESC & lhs, const D3D12_ROOT_SIGNATURE_DESC & rhs) {
		if(rhs.NumParameters > lhs.NumParameters) {
			return false;
		}

		if(rhs.NumStaticSamplers > lhs.NumStaticSamplers) {
			return false;
		}

		UINT i, j;
		for(i = 0, j = 0; i < lhs.NumParameters && j < rhs.NumParameters; ++i, ++j) {
			if(!IsCompatible(lhs.pParameters[i], rhs.pParameters[j])) {
				break;
			}
		}

		if(j != rhs.NumParameters) {
			return false;
		}

		for(i = 0, j = 0; i < lhs.NumStaticSamplers && j < rhs.NumStaticSamplers; ++i, ++j) {
			if(!IsCompatible(lhs.pStaticSamplers[i], rhs.pStaticSamplers[j])) {
				return false;
			}
		}

		return j == rhs.NumStaticSamplers;
	}

	bool RootSignatureLibrary::IsCompatible(const D3D12_DESCRIPTOR_RANGE & lhs, const D3D12_DESCRIPTOR_RANGE & rhs) {
		return lhs.NumDescriptors >= rhs.NumDescriptors &&
			lhs.BaseShaderRegister == rhs.BaseShaderRegister &&
			lhs.OffsetInDescriptorsFromTableStart == rhs.OffsetInDescriptorsFromTableStart &&
			lhs.RangeType == rhs.RangeType &&
			lhs.RegisterSpace == rhs.RegisterSpace;
	}

	bool RootSignatureLibrary::IsCompatible(const D3D12_ROOT_PARAMETER & lhs, const D3D12_ROOT_PARAMETER & rhs) {
		if(lhs.ParameterType != rhs.ParameterType || lhs.ShaderVisibility != rhs.ShaderVisibility) {
			return false;
		}

		switch(lhs.ParameterType) {
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
				return lhs.Constants.ShaderRegister == rhs.Constants.ShaderRegister &&
					lhs.Constants.RegisterSpace == rhs.Constants.RegisterSpace &&
					lhs.Constants.Num32BitValues > rhs.Constants.Num32BitValues;
			case D3D12_ROOT_PARAMETER_TYPE_SRV: [[fallthrough]];
			case D3D12_ROOT_PARAMETER_TYPE_UAV: [[fallthrough]];
			case D3D12_ROOT_PARAMETER_TYPE_CBV:
				return lhs.Descriptor.ShaderRegister == rhs.Descriptor.ShaderRegister &&
					lhs.Descriptor.RegisterSpace == rhs.Descriptor.RegisterSpace;
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				if(lhs.DescriptorTable.NumDescriptorRanges < rhs.DescriptorTable.NumDescriptorRanges) {
					return false;
				}
				UINT i = 0, j = 0;
				for(; i < lhs.DescriptorTable.NumDescriptorRanges && j < rhs.DescriptorTable.NumDescriptorRanges; ++i, ++j) {
					if(!IsCompatible(lhs.DescriptorTable.pDescriptorRanges[i], rhs.DescriptorTable.pDescriptorRanges[j])) {
						return false;
					}
				}

				return j == rhs.DescriptorTable.NumDescriptorRanges;
			}

			default:
				return false;
		}
	}

	bool RootSignatureLibrary::IsCompatible(const D3D12_DESCRIPTOR_RANGE1 & lhs, const D3D12_DESCRIPTOR_RANGE1 & rhs) {
		return lhs.NumDescriptors >= rhs.NumDescriptors &&
			lhs.BaseShaderRegister == rhs.BaseShaderRegister &&
			lhs.Flags == rhs.Flags &&
			lhs.OffsetInDescriptorsFromTableStart == rhs.OffsetInDescriptorsFromTableStart &&
			lhs.RangeType == rhs.RangeType &&
			lhs.RegisterSpace == rhs.RegisterSpace;
	}

	bool RootSignatureLibrary::IsCompatible(const D3D12_ROOT_PARAMETER1 & lhs, const D3D12_ROOT_PARAMETER1 & rhs) {
		if(lhs.ParameterType != rhs.ParameterType || lhs.ShaderVisibility != rhs.ShaderVisibility) {
			return false;
		}

		switch(lhs.ParameterType) {
			case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
				return lhs.Constants.ShaderRegister == rhs.Constants.ShaderRegister &&
					lhs.Constants.RegisterSpace == rhs.Constants.RegisterSpace &&
					lhs.Constants.Num32BitValues > rhs.Constants.Num32BitValues;
			case D3D12_ROOT_PARAMETER_TYPE_SRV: [[fallthrough]];
			case D3D12_ROOT_PARAMETER_TYPE_UAV: [[fallthrough]];
			case D3D12_ROOT_PARAMETER_TYPE_CBV:
				return lhs.Descriptor.ShaderRegister == rhs.Descriptor.ShaderRegister &&
					lhs.Descriptor.RegisterSpace == rhs.Descriptor.RegisterSpace &&
					lhs.Descriptor.Flags == rhs.Descriptor.Flags;
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				if(lhs.DescriptorTable.NumDescriptorRanges < rhs.DescriptorTable.NumDescriptorRanges) {
					return false;
				}
				UINT i = 0, j = 0;
				for(; i < lhs.DescriptorTable.NumDescriptorRanges && j < rhs.DescriptorTable.NumDescriptorRanges; ++i, ++j) {
					if(!IsCompatible(lhs.DescriptorTable.pDescriptorRanges[i], rhs.DescriptorTable.pDescriptorRanges[j])) {
						return false;
					}
				}

				return j == rhs.DescriptorTable.NumDescriptorRanges;
			}

			default:
				return false;
		}
	}

	bool RootSignatureLibrary::IsCompatible(const D3D12_STATIC_SAMPLER_DESC & lhs, const D3D12_STATIC_SAMPLER_DESC & rhs) {
		return memcmp(&lhs, &rhs, sizeof(D3D12_STATIC_SAMPLER_DESC)) == 0;
	}

	bool RootSignatureLibrary::IsCompatible(const D3D12_ROOT_SIGNATURE_DESC1 & lhs, const D3D12_ROOT_SIGNATURE_DESC1 & rhs) {
		if(rhs.NumParameters > lhs.NumParameters) {
			return false;
		}

		if(rhs.NumStaticSamplers > lhs.NumStaticSamplers) {
			return false;
		}

		UINT i, j;
		for(i = 0, j = 0; i < lhs.NumParameters && j < rhs.NumParameters; ++i, ++j) {
			if(!IsCompatible(lhs.pParameters[i], rhs.pParameters[j])) {
				break;
			}
		}

		if(j != rhs.NumParameters) {
			return false;
		}

		for(i = 0, j = 0; i < lhs.NumStaticSamplers && j < rhs.NumStaticSamplers; ++i, ++j) {
			if(!IsCompatible(lhs.pStaticSamplers[i], rhs.pStaticSamplers[j])) {
				return false;
			}
		}

		return j == rhs.NumStaticSamplers;

	}

	// does LHS provide a compatible interface for RHS

	bool RootSignatureLibrary::IsCompatible(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & lhs, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rhs) {
		if(lhs.Version != rhs.Version) {
			return false;
		}

		if(lhs.Version == D3D_ROOT_SIGNATURE_VERSION_1_0) {
			return IsCompatible(lhs.Desc_1_0, rhs.Desc_1_0);
		} else {
			return IsCompatible(lhs.Desc_1_1, rhs.Desc_1_1);
		}
	}

	RootSignatureLibrary::RootSignatureLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device) :
		objectAllocator{ allocator },
		rootSigs{ allocator },
		device { std::move(device) }
	{

	}

	Ref<DX12::RootSignatureImpl> RootSignatureLibrary::GetRootSignature(Ref<Netcode::ShaderBytecode> blobWithRootSig) {
		com_ptr<ID3D12VersionedRootSignatureDeserializer> deserializer;

		DX_API("Failed to create versioned root signature deserializer")
			D3D12CreateVersionedRootSignatureDeserializer(
				blobWithRootSig->GetBufferPointer(),
				blobWithRootSig->GetBufferSize(),
				IID_PPV_ARGS(deserializer.GetAddressOf()));

		const D3D12_VERSIONED_ROOT_SIGNATURE_DESC * desc = deserializer->GetUnconvertedRootSignatureDesc();

		for(const auto & i : rootSigs) {
			if(IsCompatible(i->GetDesc(), *desc)) {
				return i;
			}
		}

		/*
		@TODO:
		if root sig version is not supported, convert it to the max supported setting,
		serialize it and create it from that blob instead
		*/

		com_ptr<ID3D12RootSignature> rootSig;

		DX_API("Failed to create root signature")
			device->CreateRootSignature(0, blobWithRootSig->GetBufferPointer(), blobWithRootSig->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf()));

		rootSig->SetName(RootSigDebugName(blobWithRootSig->GetFileReference()).c_str());

		return rootSigs.emplace_back(std::make_shared<Netcode::Graphics::DX12::RootSignatureImpl>(std::move(rootSig), *desc));
	}

	Ref<DX12::RootSignatureImpl> RootSignatureLibrary::GetRootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & desc)
	{
		for(const auto & i : rootSigs) {
			if(IsCompatible(i->GetDesc(), desc)) {
				return i;
			}
		}
		
		com_ptr<ID3DBlob> serializedRootDesc;
		com_ptr<ID3DBlob> errorBlob;

		DX_API("Failed to serialize root signature, reason: %s", GetBlobDataAsString(errorBlob))
			D3D12SerializeVersionedRootSignature(&desc, serializedRootDesc.GetAddressOf(), errorBlob.GetAddressOf());

		com_ptr<ID3D12RootSignature> rootSig;

		DX_API("Failed to create root signature")
			device->CreateRootSignature(0, serializedRootDesc->GetBufferPointer(), serializedRootDesc->GetBufferSize(), IID_PPV_ARGS(rootSig.GetAddressOf()));

		return rootSigs.emplace_back(std::make_shared<Netcode::Graphics::DX12::RootSignatureImpl>(std::move(rootSig), desc));
	}

}

