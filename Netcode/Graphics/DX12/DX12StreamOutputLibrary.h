#pragma once

#include "DX12StreamOutput.h"

namespace Netcode::Graphics::DX12 {

	class StreamOutputLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<DX12StreamOutputRef> streamOutputs;
	public:

		StreamOutputLibrary(Memory::ObjectAllocator allocator);

		void Insert(DX12StreamOutputRef soRef);

		StreamOutputRef GetStreamOutput(const D3D12_STREAM_OUTPUT_DESC & soDesc);
	};

	using DX12StreamOutputLibrary = Netcode::Graphics::DX12::StreamOutputLibrary;
	using DX12StreamOutputLibraryRef = std::shared_ptr<DX12StreamOutputLibrary>;

}
