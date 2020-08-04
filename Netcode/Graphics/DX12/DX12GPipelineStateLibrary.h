#pragma once

#include "DX12Common.h"
#include "DX12GPipelineState.h"
#include <vector>

namespace Netcode::Graphics::DX12 {

	class GPipelineStateLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<DX12::GPipelineState>> gpsos;
		com_ptr<ID3D12Device> device;
	public:
		GPipelineStateLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device);

		Ref<PipelineState> GetGraphicsPipelineState(GPipelineStateDesc && gDesc);
	};

}
