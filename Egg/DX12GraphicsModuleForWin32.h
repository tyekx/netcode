#pragma once

#include "DX12GraphicsModule.h"

#if defined(EGG_OS_WINDOWS)
#include <Windows.h>
#endif

namespace Egg::Graphics::DX12 {

	class DX12GraphicsModuleForWin32 : public DX12GraphicsModule {
	protected:
		HWND hwnd;
	public:
		virtual void Start(Egg::Module::AApp * app) override;
		virtual void CreateSwapChain() override;
	};

}

