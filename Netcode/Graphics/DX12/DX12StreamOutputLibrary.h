#pragma once

#include "DX12StreamOutput.h"

namespace Egg::Graphics::DX12 {

	class StreamOutputLibrary {
		std::vector<DX12StreamOutputRef> streamOutputs;
	public:

		void Insert(DX12StreamOutputRef soRef);

		StreamOutputRef GetStreamOutput(const D3D12_STREAM_OUTPUT_DESC & soDesc);
	};

	using DX12StreamOutputLibrary = Egg::Graphics::DX12::StreamOutputLibrary;
	using DX12StreamOutputLibraryRef = std::shared_ptr<DX12StreamOutputLibrary>;

}
