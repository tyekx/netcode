#pragma once

#include <Netcode/HandleDecl.h>
#include "DX12Common.h"
#include <wrl.h>

namespace Netcode::Graphics::DX12 {

	class CPipelineState;
	struct CPipelineStateDesc;

	class CPipelineStateLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<CPipelineState>> cpsos;
		com_ptr<ID3D12Device> device;
	public:
		CPipelineStateLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device);

		Ref<PipelineState> GetComputePipelineState(CPipelineStateDesc && cDesc);
	};

}
