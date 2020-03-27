#pragma once

#include "DX12Common.h"
#include "DX12GPipelineState.h"
#include <vector>

namespace Netcode::Graphics::DX12 {

	class GPipelineStateLibrary {
		std::vector<DX12GPipelineStateRef> gpsos;
		com_ptr<ID3D12Device> device;
	public:
		void SetDevice(com_ptr<ID3D12Device> dev);

		PipelineStateRef GetGraphicsPipelineState(GPipelineStateDesc && gDesc);
	};

	using DX12GPipelineStateLibrary = Netcode::Graphics::DX12::GPipelineStateLibrary;
	using DX12GPipelineStateLibraryRef = std::shared_ptr<DX12GPipelineStateLibrary>;
}
