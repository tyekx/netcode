#pragma once

#include <list>
#include <memory>

namespace Egg {

	template<typename T, size_t BULK_SIZE=512>
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
			AppendFreeListItem_UnsortedImpl(item);
		}

		Bulk * head;

		void InsertBulk_UnsortedImpl(Bulk * bulk) {
			bulk->next = head;
			head = bulk;
		}

		void InsertBulk(Bulk * bulk) {
			if(head == nullptr) {
				head = bulk;
				return;
			}

			InsertBulk_UnsortedImpl(bulk);
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
