#pragma once

#include <Netcode/HandleTypes.h>
#include "DX12Common.h"
#include "DX12Includes.h"

namespace Netcode::Graphics::DX12 {

	class StreamOutputImpl : public StreamOutput {
		D3D12_STREAM_OUTPUT_DESC streamOutput;
		BuilderContainer<D3D12_SO_DECLARATION_ENTRY> declarations;
		BuilderContainer<uint32_t> strides;
	public:

		StreamOutputImpl(const D3D12_STREAM_OUTPUT_DESC & so, BuilderContainer<D3D12_SO_DECLARATION_ENTRY> soDecl, BuilderContainer<uint32_t> strides);

		const D3D12_STREAM_OUTPUT_DESC & GetNativeStreamOutput() const;

		bool operator==(const D3D12_STREAM_OUTPUT_DESC & soOutput) const;
	};

}
