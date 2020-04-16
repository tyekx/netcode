#include "DefaultModuleFactory.h"
#include "WinapiWindowModule.h"
#include "ConsoleModule.h"
#include "Network/NetcodeNetworkModule.h"
#include "Graphics/DX12/DX12GraphicsModuleForWin32.h"

namespace Netcode::Module {

	std::unique_ptr<IWindowModule> DefaultModuleFactory::CreateWindowModule(AApp * app, int windowType)  {
		switch(windowType) {
			case WINDOW_CONSOLE: return std::make_unique<Module::ConsoleModule>();
			case WINDOW_WINAPI: return std::make_unique<Module::WinapiWindowModule>();
			default: return nullptr;
		}
	}

	std::unique_ptr<IGraphicsModule> DefaultModuleFactory::CreateGraphicsModule(AApp * app, int graphicsType) {
		switch(graphicsType) {
			case GRAPHICS_DX12_WIN32: return std::make_unique<Netcode::Graphics::DX12::DX12GraphicsModuleForWin32>();
			default: return nullptr;
		}
	}

	std::unique_ptr<INetworkModule> DefaultModuleFactory::CreateNetworkModule(AApp * app, int networkType)  {
		switch(networkType) {
			case NETWORK_NETCODE: return std::make_unique<Netcode::Module::NetcodeNetworkModule>();
			default: return nullptr;
		}
		
	}

	std::unique_ptr<IAudioModule> DefaultModuleFactory::CreateAudioModule(AApp * app, int audioType) {
		return nullptr;
	}

}
