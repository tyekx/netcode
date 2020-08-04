#pragma once

#include "../../HandleTypes.h"
#include "DX12Common.h"
#include <vector>

namespace Netcode::Graphics::DX12 {

	class StreamOutput : public Netcode::StreamOutput {
		D3D12_STREAM_OUTPUT_DESC streamOutput;
		BuilderContainer<D3D12_SO_DECLARATION_ENTRY> declarations;
		BuilderContainer<UINT> strides;
	public:

		StreamOutput(const D3D12_STREAM_OUTPUT_DESC & so, BuilderContainer<D3D12_SO_DECLARATION_ENTRY> soDecl, BuilderContainer<UINT> strides);

		const D3D12_STREAM_OUTPUT_DESC & GetNativeStreamOutput() const;

		bool operator==(const D3D12_STREAM_OUTPUT_DESC & soOutput) const;
	};

}
