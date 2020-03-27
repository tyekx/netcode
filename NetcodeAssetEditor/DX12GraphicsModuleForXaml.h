#pragma once

#include <Netcode/Graphics/DX12/DX12GraphicsModule.h>

namespace Netcode::Graphics::DX12 {

	class DX12GraphicsModuleForXaml : public DX12GraphicsModule {
	public:
		virtual void CreateSwapChain() override;
	};

}
