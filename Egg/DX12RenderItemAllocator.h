#pragma once

#include "DX12RenderItem.h"

namespace Egg::Graphics::DX12 {


	struct RenderItemAllocation {
		UINT handleStart;
		RenderItem * items;
		UINT itemsLength;
	};

	class RenderItemAllocator {

		constexpr static UINT PAGE_SIZE = 512;

		struct RenderItemPage {
			UINT maxContigousBlock;
			UINT maxContigousBlockStartIndex;
			UINT numDeallocItems;
			const UINT pageId;

			RenderItem * allocatedMemory;
			UINT allocatedMemoryLength;

			RenderItemPage * nextPage;
			RenderItemPage * prevPage;

			bool HandleHostedBy(UINT globalHandle) {
				return globalHandle >= (PAGE_SIZE * pageId) && (PAGE_SIZE * (pageId + 1)) > globalHandle;
			}

			void DecrementNumDeallocatedItems() {
				// underflow protection
				if(numDeallocItems > 0) {
					numDeallocItems -= 1;
				}
			}

			void Deallocate(UINT globalHandle) {
				ASSERT(HandleHostedBy(globalHandle), "Handle is not hosted by page");

				UINT localHandle = globalHandle - PAGE_SIZE * pageId;
					
				UINT deallocatedCount = 0;
				for(UINT i = localHandle; i < PAGE_SIZE; ++i) {
					if(allocatedMemory[i].state != RenderItemState::IN_USE) {
						break;
					}

					if(allocatedMemory[i].ownerHandle != localHandle) {
						break;
					}

					allocatedMemory[i].state = RenderItemState::RETURNED;
				}

				numDeallocItems += deallocatedCount;
			}

			void SearchMaxContigousBlock() {
				// page is completely empty
				if((maxContigousBlock + numDeallocItems) == PAGE_SIZE) {
					memset(allocatedMemory, 0, allocatedMemoryLength);
					numDeallocItems = 0;
					maxContigousBlock = PAGE_SIZE;
					maxContigousBlockStartIndex = 0;
					return;
				}

				// deallocated enough items to search for a new candidate
				if(maxContigousBlock < numDeallocItems) {
					UINT candidateIndex = 0;
					UINT contigousSize = 0;
					for(UINT i = 0; i < PAGE_SIZE; ++i) {
						if(allocatedMemory[i].state == RenderItemState::IN_USE) {
							if(maxContigousBlock < contigousSize) {
								maxContigousBlock = contigousSize;
								maxContigousBlockStartIndex = candidateIndex;
							}
							candidateIndex = i + 1;
							contigousSize = 0;
						}
						++contigousSize;
					}

					if(maxContigousBlock < contigousSize) {
						maxContigousBlock = contigousSize;
						maxContigousBlockStartIndex = candidateIndex;
					}
				} else {
					// try to expand the block
					UINT h = maxContigousBlockStartIndex + maxContigousBlock;
					UINT l = maxContigousBlockStartIndex;

					for(; h < PAGE_SIZE || l > 0;) {
						RenderItemState riState = allocatedMemory[h].state;
						if(h < PAGE_SIZE) {
							if(riState != RenderItemState::IN_USE) {
								if(riState == RenderItemState::RETURNED) {
									DecrementNumDeallocatedItems();
								}
								++h;
								++maxContigousBlock;
							}
						}

						riState = allocatedMemory[l].state;

						if(l > 0) {
							if(riState != RenderItemState::IN_USE) {
								if(riState == RenderItemState::RETURNED) {
									DecrementNumDeallocatedItems();
								}
								--l;
								++maxContigousBlock;
								--maxContigousBlockStartIndex;
							}
						}
					}


				}
			}



			RenderItemAllocation Allocate(UINT numberOfItemsToAllocate) {
				ASSERT(numberOfItemsToAllocate <= maxContigousBlock, "Allocate expects the user to check if page can host");
				for(UINT i = 0; i < numberOfItemsToAllocate; ++i) {
					ASSERT(allocatedMemory[i + maxContigousBlockStartIndex].state != RenderItemState::IN_USE, "Overwriting in use memory");
					allocatedMemory[i + maxContigousBlockStartIndex].state = RenderItemState::IN_USE;
					allocatedMemory[i + maxContigousBlockStartIndex].ownerHandle = maxContigousBlockStartIndex;
				}
				
				RenderItemAllocation alloc;
				alloc.handleStart = pageId * PAGE_SIZE + maxContigousBlockStartIndex;
				alloc.items = allocatedMemory + maxContigousBlockStartIndex;
				alloc.itemsLength = numberOfItemsToAllocate;

				maxContigousBlockStartIndex += numberOfItemsToAllocate;
				INT64 underflowAvoidance = static_cast<INT64>(maxContigousBlock) - static_cast<INT64>(numberOfItemsToAllocate);
				ASSERT(underflowAvoidance >= 0, "Allocation underflows page");
				maxContigousBlock = static_cast<UINT>(underflowAvoidance);

				return alloc;
			}

			bool CanHost(UINT numberOfItemsToAllocate) {
				return maxContigousBlock >= numberOfItemsToAllocate;
			}

			RenderItemPage(UINT pageId) :
				maxContigousBlock{ PAGE_SIZE },
				maxContigousBlockStartIndex{ 0U },
				numDeallocItems{ 0U },
				pageId{ pageId },
				allocatedMemory{ static_cast<RenderItem*>(std::malloc(PAGE_SIZE * sizeof(RenderItem))) },
				allocatedMemoryLength{ static_cast<UINT>(PAGE_SIZE * sizeof(RenderItem)) },
				prevPage{ nullptr },
				nextPage{ nullptr } {

				if(allocatedMemory != nullptr) {
					memset(allocatedMemory, 0, allocatedMemoryLength);
				}
			}

			~RenderItemPage() {
				std::free(allocatedMemory);
				allocatedMemory = nullptr;
			}
		};

		RenderItemPage * startPage;
		RenderItemPage * endPage;

		UINT nextPageId;
		void InsertFirstPage() {
			startPage = new (std::nothrow) RenderItemPage(nextPageId++);

			if(startPage == nullptr) {
				__debugbreak();
			}

			endPage = startPage;
		}

		void InsertPage() {
			RenderItemPage * page = new (std::nothrow) RenderItemPage(nextPageId++);

			if(page == nullptr) {
				__debugbreak();
			}

			page->nextPage = nullptr;
			page->prevPage = endPage;
			endPage->nextPage = page;
			endPage = page;
		}

	public:
		RenderItemAllocator() : startPage{ nullptr }, endPage{ nullptr }, nextPageId{ 0U } {
			InsertFirstPage();
		}

		RenderItemAllocator(const RenderItemAllocator &) = delete;

		~RenderItemAllocator() {
			RenderItemPage * page = startPage;
			while(page != nullptr) {
				RenderItemPage * nextPage = page->nextPage;
				delete page;
				page = nextPage;
			}
		}

		RenderItem * GetAllocationFromHandle(UINT handle) {
			UINT pageId = handle / (PAGE_SIZE);

			for(RenderItemPage * i = endPage; i != nullptr; i = i->prevPage) {
				if(i->pageId == pageId) {
					return i->allocatedMemory + (handle % PAGE_SIZE);
				}
			}
			return nullptr;
		}

		RenderItemAllocation Allocate(UINT numberOfItemsToAllocate) {
			ASSERT(numberOfItemsToAllocate < PAGE_SIZE, "Cant allocate more than page_size");

			if(endPage->CanHost(numberOfItemsToAllocate)) {
				return endPage->Allocate(numberOfItemsToAllocate);
			}

			for(RenderItemPage * i = endPage; i != nullptr; i = i->prevPage) {
				if(i->CanHost(numberOfItemsToAllocate)) {
					return i->Allocate(numberOfItemsToAllocate);
				}
			}

			InsertPage();
			
			return endPage->Allocate(numberOfItemsToAllocate);
		}
	};

}

