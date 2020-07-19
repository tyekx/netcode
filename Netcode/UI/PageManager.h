#pragma once

#include "Page.h"

namespace Netcode::UI {

	class PageManager {
		Memory::ObjectAllocator controlAllocator;
		Memory::ObjectAllocator eventHandlerAllocator;
		std::vector<std::shared_ptr<Page>> pages;
		std::shared_ptr<Page> activePage;
		UInt2 windowSize;

	public:
		PageManager();

		void AddPage(std::shared_ptr<Page> page);

		void Update(float dt);

		void WindowResized(const Netcode::UInt2 & newSize);

		void Deactivate();

		void Activate(uint32_t value);

		std::shared_ptr<Page> GetPage(uint32_t value);

		void Destruct();

		template<typename PageType>
		std::shared_ptr<PageType> CreatePage(Physics::PhysX & px) {
			return controlAllocator.MakeShared<PageType>(controlAllocator, eventHandlerAllocator, px);
		}
	};

}
