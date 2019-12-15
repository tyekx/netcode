#include "Modules.h"
#include <algorithm>

namespace Egg::Module {

	void AApp::StartModule(IModule * m) {
		if(m) {
			m->Start(this);
		}
	}

	void AApp::ShutdownModule(IModule * m) {
		if(m) {
			m->Shutdown();
		}
	}

	void AApp::EventSystemChanged(AAppEventSystem * evtSys) {
		if(window) {
			evtSys->RegisterHandler(window.get());
		}

		if(graphics) {
			evtSys->RegisterHandler(graphics.get());
		}

		if(network) {
			evtSys->RegisterHandler(network.get());
		}

		if(physics) {
			evtSys->RegisterHandler(physics.get());
		}

		if(audio) {
			evtSys->RegisterHandler(audio.get());
		}
	}

	void TAppEventHandler::Focused() {
	
	}

	void TAppEventHandler::Blurred() {
	
	}

	void TAppEventHandler::DeviceLost() {

	}

	void TAppEventHandler::Resized(int width, int height) {
	
	}

	void AAppEventSystem::DeregisterHandler(TAppEventHandler * evtHandler) {
		size_t idx = 0;
		size_t size = handlers.size();

		if(size == 0) {
			return;
		}

		for(TAppEventHandler * handler : handlers) {
			if(handler == evtHandler) {
				break;
			}
			idx += 1;
		}

		if(size != idx) {
			std::iter_swap(handlers.begin() + idx, handlers.begin() + (size - 1));
		}
	}

	void AAppEventSystem::RegisterHandler(TAppEventHandler * evtHandler) {
		handlers.push_back(evtHandler);
	}
}

