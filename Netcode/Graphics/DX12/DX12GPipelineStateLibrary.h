#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Common.h"
#include <vector>
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	class GPipelineState;
	struct GPipelineStateDesc;

	class GPipelineStateLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<GPipelineState>> gpsos;
		com_ptr<ID3D12Device> device;
	public:
		GPipelineStateLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device);

		Ref<PipelineState> GetGraphicsPipelineState(GPipelineStateDesc && gDesc);
	};

}
