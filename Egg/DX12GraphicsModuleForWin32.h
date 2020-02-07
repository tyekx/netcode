#pragma once

#include "DX12GraphicsModule.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

namespace Egg::Graphics::DX12 {

	class DX12GraphicsModuleForWin32 : public DX12GraphicsModule {
	protected:
		HWND hwnd;
	public:
		virtual void Start(Egg::Module::AApp * app) override;
		virtual void CreateSwapChain() override;
	};

}

