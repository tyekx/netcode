#include "FragmentStorage.h"
#include <Netcode/Utility.h>
#include "Connection.h"

namespace Netcode::Network {
	FragmentStorage::FSItem * FragmentStorage::DReplace(FSItem ** currentHead, FSItem ** currentTail, FSItem * currentItem, FSItem * newNode) {
		if(currentItem != nullptr) {
			newNode->next = currentItem->next;
			newNode->prev = currentItem->prev;
		} else {
			newNode->next = nullptr;
			newNode->prev = nullptr;
		}

		if(*currentHead == currentItem) {
			*currentHead = newNode;
		}

		if(*currentTail == currentItem) {
			*currentTail = newNode;
		}

		if(currentItem->next != nullptr) {
			currentItem->next->prev = newNode;
		}

		if(currentItem->prev != nullptr) {
			currentItem->prev->next = newNode;
		}

		currentItem->next = nullptr;
		currentItem->prev = nullptr;

		return newNode;
	}
	bool FragmentStorage::FragmentsAreConsistent(FSItem * ptr, uint32_t * dataSize) {
		int32_t prevIndex = -1;
		uint32_t expectedSize = ptr->fragment->contentSize;
		*dataSize = 0;

		uint32_t dataSizeSum = 0;

		for(FSItem * it = ptr; it != nullptr; it = it->orderedNext) {
			int32_t currentIndex = it->fragment->header.fragmentIdx;

			if((currentIndex - prevIndex) != 1) {
				return false;
			}

			// size should never increase
			if(expectedSize < it->fragment->contentSize) {
				return false;
			}

			// MTU should be constant for a message
			if(expectedSize != it->fragment->contentSize) {
				// we allow the last one to fail this check, otherwise we delete
				if(it->orderedNext != nullptr) {
					return false;
				}
			}

			dataSizeSum += it->fragment->contentSize - NC_HEADER_SIZE;

			prevIndex = currentIndex;
		}

		*dataSize = dataSizeSum;

		return true;
	}
	GameMessage FragmentStorage::TryReassemble(FSItem * p) {
		uint32_t dataSize = 0;
		GameMessage gm;
		
		if(FragmentsAreConsistent(p, &dataSize)) {
			uint32_t requiredSpace = Utility::Align<uint32_t, 512u>(dataSize + 512u);

			Ref<NetAllocator> dstAllocator;

			if(p->linkCount != 1) {
				dstAllocator = std::make_shared<NetAllocator>(nullptr, requiredSpace);
			} else {
				dstAllocator = std::move(p->allocator);
			}

			MutableArrayView<uint8_t> reassembledBinary{ dstAllocator->MakeArray<uint8_t>(dataSize), dataSize };

			uint32_t dstOffset = 0;

			for(FSItem * iter = p; iter != nullptr; iter = iter->orderedNext) {
				const uint32_t sourceFragSize = iter->fragment->contentSize;

				memcpy(reassembledBinary.Data() + dstOffset, iter->fragment->content + NC_HEADER_SIZE, sourceFragSize - NC_HEADER_SIZE);

				dstOffset += sourceFragSize - NC_HEADER_SIZE;
			}

			const uint32_t sequence = p->fragment->header.sequence;
			gm.content = reassembledBinary;
			gm.sequence = sequence;
			gm.allocator = std::move(dstAllocator);
		}

		FSItem * it = p;
		while(it != nullptr) {
			FSItem * tmp = it->orderedNext;
			MarkDeletable(it);
			it = tmp;
		}

		return gm;
	}

	GameMessage FragmentStorage::CheckFragments() {
		Timestamp tNow = SystemClock::LocalNow();
		for(FSItem * it = orderedHead; it != nullptr; it = it->next) {
			// 1 second old fragments are way too old
			if((tNow - it->oldest) > std::chrono::seconds(1)) {
				Erase(&orderedHead, &orderedTail, it);
				MarkDeletable(it);
			} else {
				if(it->linkCount == static_cast<uint32_t>(it->fragment->header.fragmentCount)) {
					Erase(&orderedHead, &orderedTail, it);
					return TryReassemble(it);
				}
			}
		}
		return GameMessage{ };
	}

	GameMessage FragmentStorage::RunDefragmentation(FSItem * it) {
		OrderedInsert(it);

		// traverses the ordered list for possible reorders
		GameMessage gMsg = CheckFragments();

		// deallocates finalized fragments
		DeleteFragments();

		return gMsg;
	}

	GameMessage FragmentStorage::AddFragment(Ref<NetAllocator> alloc, GameFragment * fragment) {
		FSItem * node = alloc->Make<FSItem>(std::move(alloc), fragment);

		return RunDefragmentation(node);
	}
	
}
