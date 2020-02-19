#pragma once

#include <list>
#include <memory>

namespace Egg {

	template<typename T, size_t BULK_SIZE=512>
	class BulkAllocator {

		static_assert(sizeof(T) >= sizeof(void*), "Freelist need at least sizeof(void*) bytes to work properly");

		struct FreeListItem {
			FreeListItem * next;

			FreeListItem() : next{ nullptr } { }
			~FreeListItem() noexcept {
				next = nullptr;
			}
		};

		FreeListItem * freeListHead;
		FreeListItem * freeListTail;

		struct Bulk {
			T * storage;
			size_t nextIndex;

			Bulk() : storage{ nullptr }, nextIndex{ 0 } {
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

			T * Next() {
				size_t idx = nextIndex;
				nextIndex += 1;
				return storage + nextIndex;
			}

			bool CanHost() const {
				return nextIndex < BULK_SIZE;
			}
		};

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
			if(freeListHead == nullptr) {
				freeListHead = item;
				freeListTail = item;
			} else {
				freeListTail->next = item;
				freeListTail = item;
			}
		}

		Bulk * currentBulk;
		std::list<Bulk> storage;
		
		void ValidateCurrentBulk() {
			// step1: validate pointer value
			if(currentBulk == nullptr) {
				if(storage.empty()) {
					storage.emplace_back();
				}
				currentBulk = &storage.back();
			}
			
			// step2: validate content
			if(!currentBulk->CanHost()) {
				storage.emplace_back();
				currentBulk = &storage.back();
			}
		}

	public:

		template<typename ... U>
		T * Allocate(U && ... args) {
			void * ptr = GetFreeListHead();

			if(!ptr) {
				ValidateCurrentBulk();
				ptr = currentBulk->Next();
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
