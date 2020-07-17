#include "PageManager.h"

namespace Netcode::UI {

	PageManager::PageManager() : controlAllocator{ }, eventHandlerAllocator{ }, pages{  }, activePage{ nullptr } {
		pages.reserve(16);
	}

	void PageManager::AddPage(std::shared_ptr<Page> page) {
		pages.emplace_back(std::move(page));
	}

	void PageManager::Update(float dt) {
		if(activePage != nullptr) {
			activePage->Update(dt);
		}
	}

	void PageManager::WindowResized(const Netcode::UInt2 & newSize) {
		if(activePage != nullptr) {
			activePage->WindowSize(newSize);
		}
		windowSize = newSize;
	}

	void PageManager::Deactivate() {
		if(activePage != nullptr) {
			activePage->Deactivate();
		}
	}

	void PageManager::Activate(uint32_t value) {
		Netcode::UndefinedBehaviourAssertion(static_cast<uint32_t>(pages.size()) > value);
		Deactivate();
		activePage = pages[value];
		activePage->Activate();
		activePage->WindowSize(windowSize);
	}

	void PageManager::Destruct() {
		for(auto & i : pages) {
			i->Destruct();
		}
		pages.clear();
	}


}
