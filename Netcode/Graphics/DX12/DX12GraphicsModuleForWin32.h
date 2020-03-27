#pragma once

#include "DX12GraphicsModule.h"

#if defined(EGG_OS_WINDOWS)
#include <Windows.h>
#endif

namespace Netcode::Graphics::DX12 {

	class DX12GraphicsModuleForWin32 : public DX12GraphicsModule {
	protected:
		HWND hwnd;
	public:
		virtual void Start(Netcode::Module::AApp * app) override;
		virtual void CreateSwapChain() override;
	};

}

