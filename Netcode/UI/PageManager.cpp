#include "PageManager.h"

namespace Netcode::UI {

	void PageManager::SortDrawBuffer() {
		std::sort(drawBuffer, drawBuffer + numDrawnPages, [](Page * a, Page * b) -> bool {
			return a->ZIndex() < b->ZIndex();
		});
	}

	void PageManager::EraseFromDrawBuffer(uint32_t idx)
	{
		OutOfMemoryAssertion(idx < numDrawnPages);

		std::swap(drawBuffer[idx], drawBuffer[numDrawnPages - 1]);
		numDrawnPages -= 1;
	}

	void PageManager::EraseFromDrawBuffer(Page * p)
	{
		for(uint32_t i = 0; i < numDrawnPages; i++) {
			if(p == drawBuffer[i]) {
				EraseFromDrawBuffer(i);
				return;
			}
		}
		UndefinedBehaviourAssertion(true);
	}

	void PageManager::AddToDrawBuffer(Page * p)
	{
		OutOfRangeAssertion(numDrawnPages <= 8);

		for(uint32_t i = 0; i < numDrawnPages; i++) {
			if(p == drawBuffer[i]) {
				return;
			}
		}

		drawBuffer[numDrawnPages++] = p;
	}

	void PageManager::CleanDrawBuffer() {
		bool anyActive = false;
		for(uint32_t i = 0; i < numDrawnPages;) {
			if(!drawBuffer[i]->Enabled()) {
				EraseFromDrawBuffer(i);
			} else {
				anyActive |= drawBuffer[i]->IsActive();
				i++;
			}
		}

		if(!anyActive) {
			ReturnToLastPage();
		}
	}

	PageManager::PageManager() : controlAllocator{ }, eventHandlerAllocator{ }, pages{  }, drawBuffer{}, numDrawnPages{ 0 }, pageHistory{}, historyDepth{ 0 }, windowSize{ UInt2::Zero } {
		pages.reserve(16);
	}

	void PageManager::AddPage(Ref<Page> page) {
		pages.emplace_back(std::move(page));
	}

	void PageManager::Update(float dt) {
		CleanDrawBuffer();

		for(uint32_t i = 0; i < numDrawnPages; i++) {
			drawBuffer[i]->Update(dt);
		}
	}

	void PageManager::WindowResized(const Netcode::UInt2 & newSize) {
		for(auto & i : pages) {
			if(i->Enabled()) {
				i->WindowSize(newSize);
			}
		}
		windowSize = newSize;
	}

	void PageManager::Display(uint32_t idx) {
		auto page = pages[idx];
		page->ZIndex(static_cast<float>(idx));
		AddToDrawBuffer(page.get());
	}

	void PageManager::Hide(uint32_t idx) {
		Page * page = pages[idx].get();

		if(page->IsActive()) {
			page->Deactivate();
		}

		EraseFromDrawBuffer(page);
		SortDrawBuffer();
	}

	void PageManager::NavigateWithoutHistory(uint32_t idx)
	{
		for(uint32_t i = 0; i < numDrawnPages; i++) {
			drawBuffer[i]->Deactivate();
		}
		
		numDrawnPages = 0;
		historyDepth = 0;

		Display(idx);
		Activate(idx);
	}

	void PageManager::Deactivate(uint32_t value) {
		pages[value]->Deactivate();
	}

	void PageManager::NavigateTo(uint32_t idx) {
		const uint32_t groupIdx = historyDepth;

		for(uint32_t i = 0; i < numDrawnPages; i++) {
			if(drawBuffer[i]->IsActive()) {
				OutOfRangeAssertion(historyDepth <= 16);
				pageHistory[historyDepth++] = PageHistoryEntry{ drawBuffer[i], groupIdx };
				drawBuffer[i]->Deactivate();
			}
		}

		Display(idx);
		Activate(idx);
	}

	void PageManager::ReturnToLastPage()
	{
		for(uint32_t i = 0; i < numDrawnPages; ) {
			if(drawBuffer[i]->IsActive()) {
				drawBuffer[i]->Deactivate();
				EraseFromDrawBuffer(drawBuffer[i]);
			} else {
				i++;
			}
		}

		if(historyDepth == 0) {
			return;
		}

		int32_t n = static_cast<int32_t>(historyDepth - 1);
		uint32_t groupIdx = pageHistory[n].groupIdx;
		while(n >= 0 && pageHistory[n].groupIdx == groupIdx) {
			pageHistory[n].pagePtr->Activate();

			historyDepth = n;
			n--;
		}
	}

	void PageManager::Activate(uint32_t value) {
		Netcode::OutOfRangeAssertion(static_cast<uint32_t>(pages.size()) > value);
		auto v = pages[value];
		v->Activate();
		v->WindowSize(windowSize);
	}

	UInt2 PageManager::WindowSize() const
	{
		return windowSize;
	}

	void PageManager::Render(Ptr<SpriteBatch> batch)
	{
		for(uint32_t i = 0; i < numDrawnPages; i++) {
			drawBuffer[i]->Render(batch);
		}
	}

	Ref<Page> PageManager::GetPage(uint32_t value)
	{
		if(value < pages.size()) {
			return pages[value];
		}
		return nullptr;
	}

	void PageManager::Destruct() {
		for(auto & i : pages) {
			i->Destruct();
		}
		pages.clear();
	}


}
