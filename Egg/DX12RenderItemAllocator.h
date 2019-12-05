#pragma once

#include "HandleTypes.h"
#include "DX12RenderItem.h"

namespace Egg::Graphics::DX12 {

	struct RenderItemAllocation {
		HITEM handle;
		RenderItem * item;
	};

	class RenderItemAllocator {

		constexpr static UINT PAGE_SIZE = 512;

		struct RenderItemPage {
			constexpr static UINT USABLE_SIZE = PAGE_SIZE - 1U;
			UINT maxContigousBlock;
			UINT maxContigousBlockStartIndex;
			UINT numDeallocItems;
			const UINT pageId;

			RenderItem * allocatedMemory;
			UINT allocatedMemoryLength;

			bool HandleHostedBy(UINT globalHandle) {
				return globalHandle >= (USABLE_SIZE * pageId) && (USABLE_SIZE * (pageId + 1)) > globalHandle;
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
				for(UINT i = localHandle; i < USABLE_SIZE; ++i) {
					if(allocatedMemory[i].state != RenderItemState::IN_USE) {
						break;
					}

					allocatedMemory[i].state = RenderItemState::RETURNED;
				}

				numDeallocItems += deallocatedCount;
			}

			void SearchMaxContigousBlock() {
				// page is completely empty
				if((maxContigousBlock + numDeallocItems) == USABLE_SIZE) {
					memset(allocatedMemory, 0, allocatedMemoryLength);
					numDeallocItems = 0;
					maxContigousBlock = USABLE_SIZE;
					maxContigousBlockStartIndex = 0;
					return;
				}

				// deallocated enough items to search for a new candidate
				if(maxContigousBlock < numDeallocItems) {
					UINT candidateIndex = 0;
					UINT contigousSize = 0;
					for(UINT i = 0; i < USABLE_SIZE; ++i) {
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

					for(; h < USABLE_SIZE || l > 0;) {
						RenderItemState riState = allocatedMemory[h].state;
						if(h < USABLE_SIZE) {
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



			RenderItemAllocation Allocate() {
				ASSERT(allocatedMemory[maxContigousBlockStartIndex].state != RenderItemState::IN_USE, "Overwriting in use memory");

				allocatedMemory[maxContigousBlockStartIndex].state = RenderItemState::IN_USE;
				
				RenderItemAllocation alloc;
				alloc.handle = pageId * PAGE_SIZE + maxContigousBlockStartIndex;
				alloc.item = allocatedMemory + maxContigousBlockStartIndex;

				maxContigousBlockStartIndex += 1;

				INT64 underflowAvoidance = static_cast<INT64>(maxContigousBlock) - static_cast<INT64>(1);
				ASSERT(underflowAvoidance >= 0, "Allocation underflows page");
				maxContigousBlock = static_cast<UINT>(underflowAvoidance);

				return alloc;
			}

			bool CanHost() {
				return maxContigousBlock >= 1;
			}

			RenderItemPage(UINT pageId) :
				maxContigousBlock{ USABLE_SIZE },
				maxContigousBlockStartIndex{ 0U },
				numDeallocItems{ 0U },
				pageId{ pageId },
				allocatedMemory{ static_cast<RenderItem*>(std::malloc(PAGE_SIZE * sizeof(RenderItem))) },
				allocatedMemoryLength{ static_cast<UINT>(PAGE_SIZE * sizeof(RenderItem)) } {

				if(allocatedMemory != nullptr) {
					memset(allocatedMemory, 0, allocatedMemoryLength);
				}

				allocatedMemory[USABLE_SIZE].state = RenderItemState::RETURNED;
			}

			~RenderItemPage() {
				std::free(allocatedMemory);
				allocatedMemory = nullptr;
			}
		};

		RenderItemPage* pages;
		UINT nextPageId;
		UINT pagesLength;

		void InsertPage() {
			new (pages + nextPageId) RenderItemPage(nextPageId++);
		}

	public:
		RenderItemAllocator() : pages{ nullptr }, nextPageId{ 0U } {
			pagesLength = 128U;
			pages = reinterpret_cast<RenderItemPage *>(std::malloc(sizeof(RenderItemPage) * pagesLength));
			InsertPage();
		}

		RenderItemAllocator(const RenderItemAllocator &) = delete;

		~RenderItemAllocator() {
			for(UINT i = 0; i < nextPageId; ++i) {
				(pages + i)->~RenderItemPage();
			}
			std::free(pages);
		}

		RenderItem * GetAllocationFromHandle(HITEM handle) {
			UINT pageId = handle / (PAGE_SIZE);
			UINT localId = handle % (PAGE_SIZE);

			return (pages + pageId)->allocatedMemory + localId;
		}

		RenderItemAllocation Allocate() {
			for(UINT i = 0; i < nextPageId; ++i) {
				if(pages[i].CanHost()) {
					return pages[i].Allocate();
				}
			}

			InsertPage();
			
			return pages[nextPageId - 1].Allocate();
		}
	};

}

