#include "DX12StreamOutput.h"

namespace Netcode::Graphics::DX12 {

	StreamOutput::StreamOutput(const D3D12_STREAM_OUTPUT_DESC & so, BuilderContainer<D3D12_SO_DECLARATION_ENTRY> soDecl, BuilderContainer<UINT> strides) :
		streamOutput{ so }, declarations{ std::move(soDecl) }, strides{ std::move(strides) } { }

	const D3D12_STREAM_OUTPUT_DESC & StreamOutput::GetNativeStreamOutput() const {
		return streamOutput;
	}

	bool StreamOutput::operator==(const D3D12_STREAM_OUTPUT_DESC & soOutput) const {
		if(strides.size() != soOutput.NumStrides) {
			return false;
		}

		if(declarations.size() != soOutput.NumEntries) {
			return false;
		}

		for(uint32_t i = 0; i < soOutput.NumEntries; ++i) {
			if(memcmp(&declarations.at(i), &soOutput.pSODeclaration[i], sizeof(D3D12_SO_DECLARATION_ENTRY)) == 0) {
				return false;
			}
		}

		for(uint32_t i = 0; i < soOutput.NumStrides; ++i) {
			if(strides.at(i) != soOutput.pBufferStrides[i]) {
				return false;
			}
		}

		return true;
	}


}
