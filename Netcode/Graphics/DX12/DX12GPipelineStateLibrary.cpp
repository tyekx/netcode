#include "DX12GPipelineStateLibrary.h"

namespace Netcode::Graphics::DX12 {

	GPipelineStateLibrary::GPipelineStateLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device) : objectAllocator{ allocator }, gpsos { BuilderAllocator<DX12GPipelineStateRef>{ allocator} }, device{ std::move(device) }
	{
		gpsos.reserve(16);
	}

	PipelineStateRef Netcode::Graphics::DX12::GPipelineStateLibrary::GetGraphicsPipelineState(GPipelineStateDesc && gDesc) {
		for(const auto & i : gpsos) {
			if(i->GetDesc() == gDesc) {
				return i;
			}
		}

		auto pso = objectAllocator.MakeShared<DX12GPipelineState>(device.Get(), std::move(gDesc));

		gpsos.emplace_back(pso);

		return pso;
	}

}
