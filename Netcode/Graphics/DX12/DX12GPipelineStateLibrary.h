#pragma once

#include "DX12Common.h"
#include "DX12GPipelineState.h"
#include <vector>

namespace Netcode::Graphics::DX12 {

	class GPipelineStateLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<DX12GPipelineStateRef> gpsos;
		com_ptr<ID3D12Device> device;
	public:
		GPipelineStateLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device);

		PipelineStateRef GetGraphicsPipelineState(GPipelineStateDesc && gDesc);
	};

	using DX12GPipelineStateLibrary = Netcode::Graphics::DX12::GPipelineStateLibrary;
	using DX12GPipelineStateLibraryRef = std::shared_ptr<DX12GPipelineStateLibrary>;
}
