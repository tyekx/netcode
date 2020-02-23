#pragma once

#include <list>
#include <memory>

namespace Egg {

	template<typename T, size_t BULK_SIZE=512, bool FREE_LIST_SORTED=false>
	class BulkAllocator {
	public:
		static_assert(sizeof(T) >= sizeof(void*), "Freelist need at least sizeof(void*) bytes to work properly");

		struct FreeListItem {
			FreeListItem * next;

			FreeListItem() : next{ nullptr } { }
			~FreeListItem() noexcept {
				next = nullptr;
			}
		};

		struct Bulk {
			T * storage;
			Bulk * next;
			size_t nextIndex;

			Bulk() : storage{ nullptr }, next{ nullptr }, nextIndex{ 0 } {
				storage = static_cast<T *>(std::malloc(sizeof(T) * BULK_SIZE));
			}

			~Bulk() noexcept {
				std::free(storage);
				storage = 0;
				nextIndex = 0;
			}

			Bulk(const Bulk &) = delete;
			Bulk & operator=(const Bulk &) = delete;

			Bulk(Bulk &&) = default;
			Bulk & operator=(Bulk &&) = default;

			size_t GetSize() const {
				return nextIndex;
			}

			T * Next() {
				size_t idx = nextIndex;
				nextIndex += 1;
				return storage + idx;
			}

			bool CanHost() const {
				return nextIndex < BULK_SIZE;
			}
		}; 

		class Iterator {

			static_assert(FREE_LIST_SORTED, "Iterator is only allowed for sorted versions");

			FreeListItem * freeListHead;
			Bulk * bulkHead;
			size_t idx;

			bool TrySetIndex(size_t value) {
				for(T * iter = bulkHead->storage + value;
					(static_cast<void*>(iter) == static_cast<void*>(freeListHead)) && value < BULK_SIZE;
					value += 1, iter += 1, freeListHead = freeListHead->next);
				
				if(value >= bulkHead->nextIndex) {
					return false;
				}

				idx = value;

				return value != BULK_SIZE;
			}


		public:
			Iterator(FreeListItem * flHead, Bulk * bulkHead) : freeListHead{ flHead }, bulkHead{ bulkHead }, idx{ 0 } {
				while(bulkHead != nullptr && !TrySetIndex(0)) {
					bulkHead = bulkHead->next;
				}
			}

			bool operator!=(std::nullptr_t) const {
				return bulkHead != nullptr;
			}

			Iterator & operator++() {
				if(TrySetIndex(idx + 1)) {
					return *this;
				}

				do {
					bulkHead = bulkHead->next;
				} while(bulkHead != nullptr && !TrySetIndex(0));

				return *this;
			}

			T * operator->() {
				return bulkHead->storage + idx;
			}
		};

	protected:
		FreeListItem * freeListHead;
		FreeListItem * freeListTail;

		void AppendFreeListItem_UnsortedImpl(FreeListItem * item) {
			if(freeListHead == nullptr) {
				freeListHead = item;
				freeListTail = item;
			} else {
				freeListTail->next = item;
				freeListTail = item;
			}
		}

		void AppendFreeListItem_SortedImpl(FreeListItem * item) {
			// init case: freeList is empty
			if(freeListHead == nullptr) {
				freeListHead = item;
				freeListTail = item;
				item->next = nullptr;
				return;
			}
			

			// first case: item will be the next head, tail will stay in place
			if(freeListHead > item) {
				item->next = freeListHead;
				freeListHead = item;
				return;
			}

			// general case: item will be somewhere in the middle or the end
			for(FreeListItem * iter = freeListHead; iter != nullptr; iter = iter->next) {
				if(iter < item && (item < iter->next || iter->next == nullptr)) {

					if(freeListTail == iter) {
						freeListTail = item;
					}

					item->next = iter->next;
					iter->next = item;
					return;
				}
			}

#if _DEBUG
			// should never reach this code
			throw std::exception("unexpected control flow");
#endif
		}

		void * GetFreeListHead() {
			if(freeListHead == nullptr) {
				return nullptr;
			}

			FreeListItem * item = freeListHead;
			freeListHead = freeListHead->next;
			item->~FreeListItem();
			return item;
		}

		void AppendFreeListItem(FreeListItem * item) {
			if constexpr(FREE_LIST_SORTED) {
				AppendFreeListItem_SortedImpl(item);
			} else {
				AppendFreeListItem_UnsortedImpl(item);
			}
		}

		Bulk * head;

		void InsertBulk_UnsortedImpl(Bulk * bulk) {
			bulk->next = head;
			head = bulk;
		}

		void InsertBulk_SortedImpl(Bulk * bulk) {
			if(head > bulk) {
				bulk->next = head;
				head = bulk;
				return;
			}

			for(Bulk * iter = head; iter != nullptr; iter = iter->next) {
				if(iter < bulk && (bulk < iter->next || iter->next == nullptr)) {
					bulk->next = iter->next;
					iter->next = bulk;
					return;
				}
			}
		}

		void InsertBulk(Bulk * bulk) {
			if(head == nullptr) {
				head = bulk;
				return;
			}

			if constexpr(FREE_LIST_SORTED) {
				InsertBulk_SortedImpl(bulk);
			} else {
				InsertBulk_UnsortedImpl(bulk);
			}
		}

		Bulk * GetValidBulk() {
			for(Bulk * iter = head; iter != nullptr; iter = iter->next) {
				if(iter->CanHost()) {
					return iter;
				}
			}
			
			Bulk * bulk = new Bulk();

			InsertBulk(bulk);

			return bulk;
		}

	public:

		template<typename ... U>
		T * Allocate(U && ... args) {
			void * ptr = GetFreeListHead();

			if(!ptr) {
				Bulk *bulk = GetValidBulk();
				ptr = bulk->Next();
			}

			return new (ptr) T(std::forward<U>(args)...);
		}

		Iterator begin() const {
			return Iterator(freeListHead, head);
		}

		void Deallocate(T * ptr) {
			ptr->~T();

#if defined(_DEBUG)
			// debug version will zero the memory to make errors a bit more easier to catch
			memset(ptr, 0, sizeof(T));
#endif

			FreeListItem* item = reinterpret_cast<FreeListItem *>(ptr);
			
			AppendFreeListItem(new (item) FreeListItem());
		}
	};

}
