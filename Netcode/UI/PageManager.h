#pragma once

#include "Page.h"

namespace Netcode::UI {

	class PageManager {
		struct PageHistoryEntry {
			Page * pagePtr;
			uint32_t groupIdx;
		};

		Memory::ObjectAllocator controlAllocator;
		Memory::ObjectAllocator eventHandlerAllocator;
		std::vector<std::shared_ptr<Page>> pages;
		Page * drawBuffer[8];
		uint32_t numDrawnPages;
		PageHistoryEntry pageHistory[16];
		uint32_t historyDepth;
		UInt2 windowSize;

		void SortDrawBuffer();

		void EraseFromDrawBuffer(uint32_t idx);

		void EraseFromDrawBuffer(Page * p);

		void AddToDrawBuffer(Page * p);

		void CleanDrawBuffer();

	public:
		PageManager();

		void AddPage(std::shared_ptr<Page> page);

		void Update(float dt);

		void WindowResized(const Netcode::UInt2 & newSize);

		void Display(uint32_t idx);

		void Hide(uint32_t idx);

		/**
		* Activates this page as the only page that receives input
		* saves the currently opened pages in the history.
		* This page will also get displayed atop of every other
		*/
		void NavigateTo(uint32_t idx);

		/**
		* Closes the currently active pages and returns the input
		* to the page according to history. If the history is empty,
		* then it only disables the current pages
		*/
		void ReturnToLastPage();

		void Activate(uint32_t value);

		void Deactivate(uint32_t value);

		UInt2 WindowSize() const;

		void Render(Netcode::SpriteBatchPtr batch);

		void Destruct();

		std::shared_ptr<Page> GetPage(uint32_t value);

		template<typename PageType>
		std::shared_ptr<PageType> CreatePage(Physics::PhysX & px) {
			return controlAllocator.MakeShared<PageType>(controlAllocator, eventHandlerAllocator, px);
		}
	};

}
