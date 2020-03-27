#include "DX12GPipelineStateLibrary.h"

namespace Netcode::Graphics::DX12 {
	
	void GPipelineStateLibrary::SetDevice(com_ptr<ID3D12Device> dev) {
		device = std::move(dev);
	}

	PipelineStateRef Netcode::Graphics::DX12::GPipelineStateLibrary::GetGraphicsPipelineState(GPipelineStateDesc && gDesc) {
		for(const auto & i : gpsos) {
			if(i->GetDesc() == gDesc) {
				return i;
			}
		}
		return gpsos.emplace_back(std::make_shared<DX12GPipelineState>(device.Get(), std::move(gDesc)));
	}

}
