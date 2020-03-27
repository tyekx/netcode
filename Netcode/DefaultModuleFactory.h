#pragma once

#include "Modules.h"

namespace Netcode::Module {

	class DefaultModuleFactory : public IModuleFactory {
	public:
		virtual std::unique_ptr<IWindowModule> CreateWindowModule(AApp * app, int windowType) override;
		virtual std::unique_ptr<IGraphicsModule> CreateGraphicsModule(AApp * app, int graphicsType) override;
		virtual std::unique_ptr<INetworkModule> CreateNetworkModule(AApp * app, int networkType) override;
		virtual std::unique_ptr<IAudioModule> CreateAudioModule(AApp * app, int audioType) override;
	};

}
