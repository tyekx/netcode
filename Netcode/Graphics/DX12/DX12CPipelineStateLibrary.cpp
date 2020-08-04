#include "DX12CPipelineStateLibrary.h" 

namespace Netcode::Graphics::DX12 {

	CPipelineStateLibrary::CPipelineStateLibrary(Memory::ObjectAllocator allocator, com_ptr<ID3D12Device> device) :
		objectAllocator{ allocator },
		cpsos{ BuilderAllocator<Ref<DX12::CPipelineState>>(allocator) },
		device { std::move(device) }
	{
		cpsos.reserve(8);
	}

	Ref<PipelineState> CPipelineStateLibrary::GetComputePipelineState(CPipelineStateDesc && cDesc)
	{
		for(const auto & i : cpsos) {
			if(i->GetDesc() == cDesc) {
				return i;
			}
		}

		auto cpso = objectAllocator.MakeShared<DX12::CPipelineState>(device.Get(), std::move(cDesc));

		cpsos.emplace_back(cpso);

		return cpso;
	}

}
