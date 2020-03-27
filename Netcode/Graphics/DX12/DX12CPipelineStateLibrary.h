#pragma once

#include "DX12CPipelineState.h"

namespace Netcode::Graphics::DX12 {

	class CPipelineStateLibrary {
		std::vector<DX12CPipelineStateRef> cpsos;
		com_ptr<ID3D12Device> device;
	public:
		void SetDevice(com_ptr<ID3D12Device> dev);

		PipelineStateRef GetComputePipelineState(CPipelineStateDesc && cDesc);
	};

	using DX12CPipelineStateLibrary = Netcode::Graphics::DX12::CPipelineStateLibrary;
	using DX12CPipelineStateLibraryRef = std::shared_ptr<DX12CPipelineStateLibrary>;

}
