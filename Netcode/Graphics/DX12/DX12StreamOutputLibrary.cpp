#include "DX12StreamOutputLibrary.h"
#include "DX12Includes.h"
#include "DX12StreamOutput.h"
#include <algorithm>

namespace Netcode::Graphics::DX12 {
	StreamOutputLibrary::StreamOutputLibrary(Memory::ObjectAllocator allocator) : objectAllocator{ allocator },
		streamOutputs{ BuilderAllocator<Ref<StreamOutputImpl>>{ allocator } }
	{
		streamOutputs.reserve(8);
	}

	void StreamOutputLibrary::Insert(Ref<StreamOutputImpl> soRef) {
		streamOutputs.push_back(std::move(soRef));
	}

	Ref<StreamOutputImpl> StreamOutputLibrary::GetStreamOutput(const D3D12_STREAM_OUTPUT_DESC & soDesc) {
		auto it = std::find_if(std::begin(streamOutputs), std::end(streamOutputs), [&soDesc](const Ref<StreamOutputImpl> & item) -> bool {
			return *item == soDesc;
		});

		if(it == std::end(streamOutputs)) {
			return nullptr;
		}

		return *it;
	}

}
