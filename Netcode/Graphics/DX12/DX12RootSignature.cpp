#include "DX12RootSignature.h"
#include "../../Common.h"

namespace Egg::Graphics::DX12 {
	
	void RootSignatureDesc::CreateRS_1_0(const D3D12_ROOT_SIGNATURE_DESC & rsDesc) {
		size_t size = 0;
		size_t paramsSize = rsDesc.NumParameters * sizeof(D3D12_ROOT_PARAMETER);
		size_t samplersSize = rsDesc.NumStaticSamplers * sizeof(D3D12_STATIC_SAMPLER_DESC);

		size += paramsSize;
		size += samplersSize;

		for(uint32_t i = 0; i < rsDesc.NumParameters; ++i) {
			if(rsDesc.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
				size += rsDesc.pParameters[i].DescriptorTable.NumDescriptorRanges * sizeof(D3D12_DESCRIPTOR_RANGE);
			}
		}

		buffer = std::make_unique<uint8_t[]>(size);

		D3D12_ROOT_PARAMETER * params = reinterpret_cast<D3D12_ROOT_PARAMETER *>(buffer.get());
		D3D12_STATIC_SAMPLER_DESC * samplers = reinterpret_cast<D3D12_STATIC_SAMPLER_DESC *>(buffer.get() + paramsSize);
		D3D12_DESCRIPTOR_RANGE * ranges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE *>(buffer.get() + paramsSize + samplersSize);

		memcpy(params, rsDesc.pParameters, paramsSize);
		memcpy(samplers, rsDesc.pStaticSamplers, samplersSize);

		for(uint32_t i = 0; i < rsDesc.NumParameters; ++i) {
			if(rsDesc.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
				memcpy(ranges, rsDesc.pParameters[i].DescriptorTable.pDescriptorRanges, rsDesc.pParameters[i].DescriptorTable.NumDescriptorRanges * sizeof(D3D12_DESCRIPTOR_RANGE));
				ranges += rsDesc.pParameters[i].DescriptorTable.NumDescriptorRanges;
			}
		}
		rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
		rootSigDesc.Desc_1_0.Flags = rsDesc.Flags;
		rootSigDesc.Desc_1_0.NumParameters = rsDesc.NumParameters;
		rootSigDesc.Desc_1_0.pParameters = params;
		rootSigDesc.Desc_1_0.NumStaticSamplers = rsDesc.NumStaticSamplers;
		rootSigDesc.Desc_1_0.pStaticSamplers = samplers;
	}
	void RootSignatureDesc::CreateRS_1_1(const D3D12_ROOT_SIGNATURE_DESC1 & rsDesc) {
		size_t size = 0;
		size_t paramsSize = rsDesc.NumParameters * sizeof(D3D12_ROOT_PARAMETER1);
		size_t samplersSize = rsDesc.NumStaticSamplers * sizeof(D3D12_STATIC_SAMPLER_DESC);

		size += paramsSize;
		size += samplersSize;

		for(uint32_t i = 0; i < rsDesc.NumParameters; ++i) {
			if(rsDesc.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
				size += rsDesc.pParameters[i].DescriptorTable.NumDescriptorRanges * sizeof(D3D12_DESCRIPTOR_RANGE1);
			}
		}

		buffer = std::make_unique<uint8_t[]>(size);

		D3D12_ROOT_PARAMETER1 * params = reinterpret_cast<D3D12_ROOT_PARAMETER1 *>(buffer.get());
		D3D12_STATIC_SAMPLER_DESC * samplers = reinterpret_cast<D3D12_STATIC_SAMPLER_DESC *>(buffer.get() + paramsSize);
		D3D12_DESCRIPTOR_RANGE1 * ranges = reinterpret_cast<D3D12_DESCRIPTOR_RANGE1 *>(buffer.get() + paramsSize + samplersSize);

		memcpy(params, rsDesc.pParameters, paramsSize);
		memcpy(samplers, rsDesc.pStaticSamplers, samplersSize);

		for(uint32_t i = 0; i < rsDesc.NumParameters; ++i) {
			if(rsDesc.pParameters[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
				memcpy(ranges, rsDesc.pParameters[i].DescriptorTable.pDescriptorRanges, rsDesc.pParameters[i].DescriptorTable.NumDescriptorRanges * sizeof(D3D12_DESCRIPTOR_RANGE1));
				ranges += rsDesc.pParameters[i].DescriptorTable.NumDescriptorRanges;
			}
		}
		rootSigDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		rootSigDesc.Desc_1_1.Flags = rsDesc.Flags;
		rootSigDesc.Desc_1_1.NumParameters = rsDesc.NumParameters;
		rootSigDesc.Desc_1_1.pParameters = params;
		rootSigDesc.Desc_1_1.NumStaticSamplers = rsDesc.NumStaticSamplers;
		rootSigDesc.Desc_1_1.pStaticSamplers = samplers;
	}

	RootSignatureDesc::RootSignatureDesc(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rsDesc) : buffer{ nullptr }, rootSigDesc{} {
		if(rsDesc.Version == D3D_ROOT_SIGNATURE_VERSION_1_0) {
			CreateRS_1_0(rsDesc.Desc_1_0);
		} else if(rsDesc.Version == D3D_ROOT_SIGNATURE_VERSION_1_1) {
			CreateRS_1_1(rsDesc.Desc_1_1);
		} else {
			ASSERT(false, "Unknown root signature version");
		}
	}

	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & RootSignatureDesc::GetNativeDesc() const {
		return rootSigDesc;
	}


	RootSignature::RootSignature(com_ptr<ID3D12RootSignature> rs, const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & rsDesc) : rootSignature{ std::move(rs) }, desc{ rsDesc } { }

	const D3D12_VERSIONED_ROOT_SIGNATURE_DESC & RootSignature::GetDesc() const {
		return desc.GetNativeDesc();
	}

	ID3D12RootSignature * RootSignature::GetNativeRootSignature() const {
		return rootSignature.Get();
	}

	void * RootSignature::GetImplDetail() const {
		return GetNativeRootSignature();
	}

}

