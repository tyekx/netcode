#pragma once

#include "DX12CPipelineState.h"

namespace Netcode::Graphics::DX12 {

	class CPipelineStateLibrary {
		Memory::ObjectAllocator objectAllocator;
		BuilderContainer<Ref<DX12::CPipelineState>> cpsos;
		com_ptr<ID3D12Device> device;
	public:
		CPipelineStateLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device);

		Ref<Netcode::PipelineState> GetComputePipelineState(CPipelineStateDesc && cDesc);
	};

	using DX12CPipelineStateLibrary = Netcode::Graphics::DX12::CPipelineStateLibrary;
	using DX12CPipelineStateLibraryRef = std::shared_ptr<DX12CPipelineStateLibrary>;

}
