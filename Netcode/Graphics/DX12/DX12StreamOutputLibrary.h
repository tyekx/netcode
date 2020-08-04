#pragma once

#include "DX12StreamOutput.h"

namespace Netcode::Graphics::DX12 {

	class StreamOutputLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<DX12::StreamOutput>> streamOutputs;
	public:

		StreamOutputLibrary(Memory::ObjectAllocator allocator);

		void Insert(Ref<DX12::StreamOutput> soRef);

		Ref<Netcode::StreamOutput> GetStreamOutput(const D3D12_STREAM_OUTPUT_DESC & soDesc);
	};

}
