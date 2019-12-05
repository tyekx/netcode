#include "DefaultModuleFactory.h"
#include "WinapiWindowModule.h"
#include "DX12GraphicsModule.h"

namespace Egg::Module {

	std::unique_ptr<IWindowModule> DefaultModuleFactory::CreateWindowModule(AApp * app, int windowType)  {
		return std::make_unique<Module::WinapiWindowModule>();
	}

	std::unique_ptr<IGraphicsModule> DefaultModuleFactory::CreateGraphicsModule(AApp * app, int graphicsType) {
		return std::make_unique<Egg::Graphics::DX12::DX12GraphicsModule>();
	}

	std::unique_ptr<INetworkModule> DefaultModuleFactory::CreateNetworkModule(AApp * app, int networkType)  {
		return nullptr;
	}

	std::unique_ptr<IAudioModule> DefaultModuleFactory::CreateAudioModule(AApp * app, int audioType) {
		return nullptr;
	}

	std::unique_ptr<IPhysicsModule> DefaultModuleFactory::CreatePhysicsModule(AApp * app, int physicsType) {
		return nullptr;
	}

}
