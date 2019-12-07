#pragma once

#include "HandleTypes.h"
#include "DX12RenderItem.h"
#include "DX12RenderItemAllocator.h"

namespace Egg::Graphics::DX12 {

	class RenderItemCollection {

		RenderItemAllocator allocator;

	public:
		HITEM CreateItem() {
			auto allocation = allocator.Allocate();

			new (allocation.item) RenderItem();
			allocation.item->state = RenderItemState::IN_USE;

			return allocation.handle;
		}

		RenderItem * GetItem(HITEM item) {
			return allocator.GetAllocationFromHandle(item);
		}

	};

}
