#pragma once

#include "HandleTypes.h"
#include "DX12Common.h"
#include <vector>

namespace Egg::Graphics::DX12 {

	class StreamOutput : public Egg::StreamOutput {
		D3D12_STREAM_OUTPUT_DESC streamOutput;
		std::vector<D3D12_SO_DECLARATION_ENTRY> declarations;
		std::vector<UINT> strides;
	public:

		StreamOutput(const D3D12_STREAM_OUTPUT_DESC & so, std::vector<D3D12_SO_DECLARATION_ENTRY> && soDecl, std::vector<UINT> && strides);

		const D3D12_STREAM_OUTPUT_DESC & GetNativeStreamOutput() const;

		bool operator==(const D3D12_STREAM_OUTPUT_DESC & soOutput) const;
	};

	using DX12StreamOutput = Egg::Graphics::DX12::StreamOutput;
	using DX12StreamOutputRef = std::shared_ptr<DX12StreamOutput>;

}
