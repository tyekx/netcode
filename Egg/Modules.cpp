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

	void AApp::AddAppEventHandlers(AppEventSystem * evtSys) {
		if(window) {
			evtSys->AddHandler(window.get());
		}

		if(graphics) {
			evtSys->AddHandler(graphics.get());
		}

		if(network) {
			evtSys->AddHandler(network.get());
		}

		if(physics) {
			evtSys->AddHandler(physics.get());
		}

		if(audio) {
			evtSys->AddHandler(audio.get());
		}
	}

	void AppEventSystem::RemoveHandler(IAppEventHandler * evtHandler) {
		size_t idx = 0;
		size_t size = handlers.size();

		if(size == 0) {
			return;
		}

		for(IAppEventHandler * handler : handlers) {
			if(handler == evtHandler) {
				break;
			}
			idx += 1;
		}

		if(size != idx) {
			std::iter_swap(handlers.begin() + idx, handlers.begin() + (size - 1));
		}
	}

	void AppEventSystem::AddHandler(IAppEventHandler * evtHandler) {
		handlers.push_back(evtHandler);
	}

	void AppEventSystem::Dispatch() {
		for(const AppEvent & e : events) {
			Broadcast(e);
		}
		events.clear();
	}

	void AppEventSystem::PostEvent(const AppEvent & evt) {
		events.push_back(evt);
	}

	void AppEventSystem::Broadcast(const AppEvent & evt) {
		for(IAppEventHandler * handler : handlers) {
			handler->HandleEvent(evt);
		}
	}

	void TAppEventHandler::HandleEvent(const AppEvent & evt) {
		switch(evt.type) {
		case EAppEventType::RESIZED:
			OnResized(evt.resizeArgs.x, evt.resizeArgs.y);
			return;
		case EAppEventType::BLURRED:
			OnBlur();
			return;
		case EAppEventType::CLOSED:
			OnClosed();
			return;
		case EAppEventType::FOCUSED:
			OnFocus();
			return;
		case EAppEventType::DEVICE_LOST:
			OnDeviceLost();
			return;
		}
	}

	void TAppEventHandler::OnDeviceLost() {

	}

	void TAppEventHandler::OnBlur() {

	}

	void TAppEventHandler::OnFocus() {

	}

	void TAppEventHandler::OnResized(int x, int y) {

	}

	void TAppEventHandler::OnClosed() {

	}
}

