#pragma once

#include "Modules.h"

#define WINDOW_WINAPI (0)
#define WINDOW_CONSOLE (1)

#define GRAPHICS_DX12_WIN32 (0)

#define NETWORK_NETCODE (0)

namespace Netcode::Module {

	class DefaultModuleFactory : public IModuleFactory {
	public:
		virtual std::unique_ptr<IWindowModule> CreateWindowModule(AApp * app, int windowType) override;
		virtual std::unique_ptr<IGraphicsModule> CreateGraphicsModule(AApp * app, int graphicsType) override;
		virtual std::unique_ptr<INetworkModule> CreateNetworkModule(AApp * app, int networkType) override;
		virtual std::unique_ptr<IAudioModule> CreateAudioModule(AApp * app, int audioType) override;
	};

}
