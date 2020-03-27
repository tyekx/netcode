#include "DX12CPipelineStateLibrary.h" 

namespace Netcode::Graphics::DX12 {

	void CPipelineStateLibrary::SetDevice(com_ptr<ID3D12Device> dev)
	{
		device = std::move(dev);
	}

	PipelineStateRef CPipelineStateLibrary::GetComputePipelineState(CPipelineStateDesc && cDesc)
	{
		for(const auto & i : cpsos) {
			if(i->GetDesc() == cDesc) {
				return i;
			}
		}
		return cpsos.emplace_back(std::make_shared<DX12CPipelineState>(device.Get(), std::move(cDesc)));
	}

}
