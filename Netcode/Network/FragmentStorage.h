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

	struct GameMessage;
	
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

		FSItem * DReplace(FSItem ** currentHead, FSItem ** currentTail, FSItem * currentItem, FSItem * newNode);

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

		bool FragmentsAreConsistent(FSItem * ptr, uint32_t * dataSize);

		GameMessage TryReassemble(FSItem * p);

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

		GameMessage CheckFragments();

		GameMessage RunDefragmentation(FSItem * it);
	public:
		FragmentStorage() : orderedHead{ nullptr }, orderedTail{ nullptr }, deletableHead{ nullptr } { }

		GameMessage AddFragment(Ref<NetAllocator> alloc, GameFragment * fragment);
		
	};
	
}
