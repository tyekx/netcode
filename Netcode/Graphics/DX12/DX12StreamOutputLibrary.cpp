#include "DX12StreamOutputLibrary.h"
#include <algorithm>

namespace Netcode::Graphics::DX12 {
	StreamOutputLibrary::StreamOutputLibrary(Memory::ObjectAllocator allocator) : objectAllocator{ allocator },
		streamOutputs{ BuilderAllocator<Ref<DX12::StreamOutput>>{ allocator } }
	{
		streamOutputs.reserve(8);
	}

	void StreamOutputLibrary::Insert(Ref<DX12::StreamOutput> soRef) {
		streamOutputs.push_back(std::move(soRef));
	}

	Ref<Netcode::StreamOutput> StreamOutputLibrary::GetStreamOutput(const D3D12_STREAM_OUTPUT_DESC & soDesc) {
		auto it = std::find_if(std::begin(streamOutputs), std::end(streamOutputs), [&soDesc](const Ref<DX12::StreamOutput> & item) -> bool {
			return *item == soDesc;
		});

		if(it == std::end(streamOutputs)) {
			return nullptr;
		}

		return *it;
	}

}
