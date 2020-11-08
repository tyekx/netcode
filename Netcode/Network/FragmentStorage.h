#pragma once

#include <Netcode/HandleDecl.h>
#include <Netcode/Network/NetAllocator.h>
#include "Dtls.h"

namespace Netcode::Network {

	constexpr static uint32_t NC_HEADER_SIZE = 6;

#pragma pack(push, 1)
	struct NcCommonHeaderWire {
		union {
			uint8_t firstByte;
			uint8_t bytes[4];
			uint32_t rawData;
		};
		uint8_t index;
		uint8_t count;
	};
#pragma pack(pop)

	struct NcGameHeader {
		uint32_t sequence;
		uint16_t fragmentIdx;
		uint16_t fragmentCount;

		void Store(NcCommonHeaderWire * wireData) {
			*reinterpret_cast<uint32_t *>(&wireData->firstByte) = htonl(sequence);
			wireData->firstByte |= 0x80;
			wireData->index = fragmentIdx;
			wireData->count = static_cast<uint8_t>(fragmentCount - 1);
		}

		static NcGameHeader Load(const NcCommonHeaderWire * wireData) {
			NcGameHeader h;
			h.sequence = ntohl(wireData->rawData) & 0x7FFFFFFFu;
			h.fragmentIdx = wireData->index;
			h.fragmentCount = static_cast<uint16_t>(wireData->count) + 1;
			return h;
		}
	};

	struct GameFragment {
		DtlsRecordLayer record;
		NcGameHeader header;
		uint32_t contentSize;
		uint8_t * content;
		UdpPacket * packet;
	};

	class FragmentStorage {

		struct FSItem {
			Ref<NetAllocator> allocator;
			GameFragment * fragment;
			FSItem * orderedNext;
			FSItem * next;
			FSItem * prev;
			Timestamp oldest;
			uint16_t linkCount;

			FSItem(Ref<NetAllocator> alloc, GameFragment * fragment) :
				allocator{ std::move(alloc) }, fragment{ fragment },
				orderedNext{ nullptr }, next{ nullptr }, prev{ nullptr }, oldest{ fragment->packet->timestamp }, linkCount{ 1 } {}

			bool IsMatching(const FSItem & rhs) const {
				return fragment->header.sequence == rhs.fragment->header.sequence &&
					fragment->packet->endpoint == rhs.fragment->packet->endpoint &&
					fragment->header.fragmentCount == rhs.fragment->header.fragmentCount;
			}

			bool operator==(const FSItem & rhs) const noexcept {
				return fragment->header.fragmentIdx == rhs.fragment->header.fragmentIdx;
			}

			bool operator<(const FSItem & rhs) const noexcept {
				return fragment->header.fragmentIdx < rhs.fragment->header.fragmentIdx;
			}
		};

		FSItem * orderedHead;
		FSItem * orderedTail;
		FSItem * deletableHead;

		void MarkDeletable(FSItem * node) {
			node->next = deletableHead;
			node->prev = nullptr;
			deletableHead = node;
		}

		void Erase(FSItem ** currentHead, FSItem ** currentTail, FSItem * node) {
			if(*currentHead == node) {
				*currentHead = node->next;
			}

			if(*currentTail == node) {
				*currentTail = node->prev;
			}

			FSItem * l = node->prev;
			FSItem * r = node->next;

			if(r != nullptr) {
				r->prev = l;
			}

			if(l != nullptr) {
				l->next = r;
			}
		}

		FSItem * DReplace(FSItem ** currentHead, FSItem ** currentTail, FSItem * currentItem, FSItem * newNode) {
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

		void PushFront(FSItem ** currentHead, FSItem ** currentTail, FSItem * node) {
			if(*currentHead == nullptr) {
				*currentHead = node;
				*currentTail = node;
				return;
			}

			node->prev = nullptr;
			node->next = (*currentHead);
			(*currentHead)->prev = node;
			*currentHead = node;
		}

		void OrderedInsert(FSItem * item) {
			for(FSItem * iter = orderedHead; iter != nullptr; iter = iter->next) {
				if(iter->IsMatching(*item)) {
					if(*iter == *item) {
						return MarkDeletable(item);
					}

					if(*item < *iter) {
						DReplace(&orderedHead, &orderedTail, iter, item);
						// keep the old timestamp for the new head
						item->oldest = iter->oldest;
						item->orderedNext = iter;
						item->linkCount += iter->linkCount;
					} else {
						FSItem * p = iter;

						for(FSItem * i2 = iter->orderedNext; i2 != nullptr; i2 = i2->orderedNext) {
							if(*i2 == *item) {
								return MarkDeletable(item);
							}

							if(*p < *item && *item < *i2) {
								item->orderedNext = i2;
								break;
							}
							p = i2;
						}

						iter->linkCount += 1;
						p->orderedNext = item;
					}
					return;
				}
			}

			PushFront(&orderedHead, &orderedTail, item);
		}

		bool FragmentsAreConsistent(FSItem * ptr, uint32_t * dataSize) {
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

				dataSizeSum += it->fragment->contentSize;

				prevIndex = currentIndex;
			}

			*dataSize = dataSizeSum;

			return true;
		}

		void TryReassemble(FSItem * p);

		void DeleteFragments() {
			FSItem * it = deletableHead;
			while(it != nullptr) {
				FSItem * tmp = it->next;
				Ref<NetAllocator> alloc = std::move(it->allocator);
				alloc.reset();
				it = tmp;
			}
			deletableHead = nullptr;
		}

		void CheckFragments() {
			Timestamp tNow = SystemClock::LocalNow();
			for(FSItem * it = orderedHead; it != nullptr; it = it->next) {
				// 1 second old fragments are way too old
				if((tNow - it->oldest) > std::chrono::seconds(1)) {
					Erase(&orderedHead, &orderedTail, it);
					MarkDeletable(it);
				} else {
					if(it->linkCount == static_cast<uint32_t>(it->fragment->header.fragmentCount)) {
						Erase(&orderedHead, &orderedTail, it);
						TryReassemble(it);
					}
				}
			}
		}

		void RunDefragmentation(FSItem * it) {
			OrderedInsert(it);

			// traverses the ordered list for possible reorders
			CheckFragments();

			// deallocates finalized fragments
			DeleteFragments();
		}
	public:
		FragmentStorage() : orderedHead{ nullptr }, orderedTail{ nullptr }, deletableHead{ nullptr } { }

		void AddFragment(Ref<NetAllocator> alloc, GameFragment * fragment) {
			FSItem * node = alloc->Make<FSItem>(std::move(alloc), fragment);

			RunDefragmentation(node);
		}
		
	};
	
}
