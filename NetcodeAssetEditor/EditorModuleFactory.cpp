#include "pch.h"
#include "EditorModuleFactory.h"
#include "DX12GraphicsModuleForXaml.h"

namespace Netcode::Module {
	std::unique_ptr<IGraphicsModule> EditorModuleFactory::CreateGraphicsModule(AApp * app, int graphicsType)
	{
		return std::make_unique<Netcode::Graphics::DX12::DX12GraphicsModuleForXaml>();
	}
}

