#include "DX12StreamOutputLibrary.h"
#include <algorithm>

namespace Netcode::Graphics::DX12 {
	StreamOutputLibrary::StreamOutputLibrary(Memory::ObjectAllocator allocator) : objectAllocator{ allocator },
		streamOutputs{ BuilderAllocator<DX12StreamOutputRef>{ allocator } }
	{
		streamOutputs.reserve(8);
	}

	void StreamOutputLibrary::Insert(DX12StreamOutputRef soRef) {
		streamOutputs.push_back(std::move(soRef));
	}

	StreamOutputRef StreamOutputLibrary::GetStreamOutput(const D3D12_STREAM_OUTPUT_DESC & soDesc) {
		auto it = std::find_if(std::begin(streamOutputs), std::end(streamOutputs), [&soDesc](const DX12StreamOutputRef & item) -> bool {
			return *item == soDesc;
		});

		if(it == std::end(streamOutputs)) {
			return nullptr;
		}

		return *it;
	}

}
