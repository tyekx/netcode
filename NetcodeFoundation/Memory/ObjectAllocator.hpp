#pragma once

#include "Common.h"
#include "StdAllocatorAdapter.hpp"

#include <vector>
#include <memory>

namespace Netcode::Memory {

    class ObjectAllocator {

        inline static uint64_t GetAlignmentCorrection(const void * ptr, size_t headSize, uint64_t alignment) {
            const uintptr_t pRaw = reinterpret_cast<uintptr_t>(ptr);
            const uintptr_t pAligned = Align(pRaw + headSize, alignment);
            return pAligned - pRaw;
        }

        struct AllocMetaData {
            uint64_t correction : 8;
            uint64_t numElements : 56;

            AllocMetaData() noexcept : correction{ 0 }, numElements{ 0 } { }
            AllocMetaData(uint8_t correction, uint64_t numElements) noexcept : correction{ correction }, numElements{ numElements } { }
            ~AllocMetaData() noexcept = default;
        };

        constexpr static size_t BACK_ALIGNMENT = 2 * sizeof(void *);

        static_assert(sizeof(AllocMetaData) == sizeof(uint64_t), "ABI error");

        struct FreeListItem {
            size_t numBytes;
            FreeListItem * next;
            FreeListItem * prev;

            FreeListItem() noexcept = default;

            ~FreeListItem() noexcept {
                next = nullptr;
                prev = nullptr;
                numBytes = 0;
            }

            void * AddressOf() {
                return this;
            }

            bool CanFit(size_t totalAllocationSize) const {
                return totalAllocationSize == numBytes ||
                    (totalAllocationSize <= (numBytes - sizeof(FreeListItem)));
            }

            size_t GetTotalAllocationSize(size_t headSize, size_t dataSize, size_t dataAlignment, size_t correction) const {
                uintptr_t pRaw = reinterpret_cast<uintptr_t>(this);

                uintptr_t pEnd = Align(pRaw + headSize + correction + dataSize, BACK_ALIGNMENT);

                return static_cast<size_t>(pEnd - pRaw);
            }

            size_t GetCorrectionSize(size_t headSize, size_t dataAlignment) const {
                uintptr_t pRaw = reinterpret_cast<uintptr_t>(this);

                uintptr_t pValue = pRaw + headSize;

                uintptr_t pAligned = Align(pValue, dataAlignment);

                return static_cast<size_t>(pAligned - pValue);
            }
        };

        struct PageHeader {
            uint8_t * data;
            size_t offset;
            size_t numBytes;

            PageHeader(uint8_t * pData, size_t numBytes) : data{ pData }, offset{ 0 }, numBytes{ numBytes } { }
        };

        class FreeList {
            FreeListItem * head;
            FreeListItem * tail;
            FreeListItem * defragLeft;
            FreeListItem * defragRight;

            void ResetDefragPointers() {
                defragLeft = head;
                defragRight = head;
            }

            void RemoveIgnoreCache(FreeListItem * item) {
                if(item->prev != nullptr) {
                    item->prev->next = item->next;
                } else {
                    head = item->next;
                }

                if(item->next != nullptr) {
                    item->next->prev = item->prev;
                } else {
                    tail = item->prev;
                }

                item->next = nullptr;
                item->prev = nullptr;
            }

        public:
            FreeList() : head{ nullptr }, tail{ nullptr }, defragLeft{ nullptr }, defragRight{ nullptr } { }

            /*
            Defragmentation with progress. Tries to merge consequtive free'd memory blocks together
            * @param numSteps limit the progression of the defragmentation
            * @return the steps taken [0, numSteps]
            */
            uint64_t Defragment(uint64_t numSteps, PageHeader & pageHeader) {
                if(head == tail) {
                    return 0;
                }

                if(defragLeft == nullptr || defragRight == nullptr) {
                    ResetDefragPointers();
                }

                size_t i;

                const uint8_t * pagePtr = pageHeader.data + pageHeader.offset;

                for(i = 0; i < numSteps && defragRight != nullptr && defragLeft != nullptr; ++i) {
                    const uint8_t * leftPtr = reinterpret_cast<const uint8_t *>(defragLeft->AddressOf());
                    const uint8_t * rightPtr = reinterpret_cast<const uint8_t *>(defragRight->AddressOf());

                    if((rightPtr + defragRight->numBytes) == pagePtr) {
                        pagePtr -= defragRight->numBytes;
                        pageHeader.offset -= defragRight->numBytes;
                        RemoveIgnoreCache(defragRight);
                        Destroy(defragRight);
                        ResetDefragPointers();
                        continue;
                    }

                    if((leftPtr + defragLeft->numBytes) == rightPtr) {
                        defragLeft->numBytes += defragRight->numBytes;
                        RemoveIgnoreCache(defragRight);
                        Destroy(defragRight);
                        ResetDefragPointers();
                        continue;
                    }

                    if(defragRight == tail) {
                        defragLeft = defragLeft->next;
                        defragRight = head;
                    } else {
                        defragRight = defragRight->next;
                    }
                }

                return i;
            }

            // sorted insertion
            void Insert(FreeListItem * item) {
                if(head == nullptr) {
                    head = item;
                    tail = item;
                    item->next = item->prev = nullptr;
                    return;
                }

                // case1: start of the list
                if(head->numBytes <= item->numBytes) {
                    item->prev = nullptr;
                    item->next = head;
                    head->prev = item;
                    head = item;
                    return;
                }

                // case2: end of the list
                if(tail->numBytes >= item->numBytes) {
                    item->next = nullptr;
                    item->prev = tail;
                    tail->next = item;
                    tail = item;
                    return;
                }

                // case3: only 1 element in the free list, handled by case1+case2 together

                // case4: insert between 2 items
                for(FreeListItem * it = head->next; it != nullptr; it = it->next) {
                    if(it->prev->numBytes >= item->numBytes && it->numBytes <= item->numBytes) {
                        it->prev->next = item;
                        item->next = it;
                        item->prev = it->prev;
                        it->prev = item;
                        return;
                    }
                }

                UndefinedBehaviourAssertion(false);
            }

            void Remove(FreeListItem * item) {
                if(defragRight == item) {
                    defragRight = item->next;
                }

                if(defragLeft == item) {
                    defragLeft = item->next;
                }

                RemoveIgnoreCache(item);
            }

            FreeListItem * Head() {
                return head;
            }
        };

        class Page {
        public:
            PageHeader header;
            FreeList freeList;

            Page(uint8_t * pData, size_t numBytes) : header{ pData, numBytes }, freeList{  } { }

            bool Owns(const void * ptr) const {
                ptrdiff_t diff = reinterpret_cast<const uint8_t *>(ptr) - static_cast<const uint8_t *>(header.data);

                return diff >= 0 && diff < header.numBytes;
            }

            uint64_t Defragment(uint64_t numSteps) {
                return freeList.Defragment(numSteps, header);
            }

            template<typename T>
            void Deallocate(T * pData, size_t n) {
                if(pData == nullptr) {
                    return;
                }

                AllocMetaData * allocInfo = (reinterpret_cast<AllocMetaData *>(pData) - 1);
                size_t numElements = allocInfo->numElements;
                size_t correction = allocInfo->correction;

                void * actualStart = reinterpret_cast<uint8_t *>(Destroy<AllocMetaData>(allocInfo)) - correction;
                size_t blockSize = correction + sizeof(AllocMetaData) + Align<size_t>(sizeof(T) * numElements, BACK_ALIGNMENT);

                FreeListItem * item = Reconstruct<void, FreeListItem>(&actualStart);
                item->numBytes = blockSize;

                freeList.Insert(item);
            }

            inline bool HasEnoughPristineSpace(size_t sizeInBytes) const {
                return (header.numBytes - header.offset) >= sizeInBytes;
            }

            template<typename T>
            MemoryBlock Allocate(size_t n, uint64_t alignment = 16) {

                // assert alignment is pow2
                OutOfRangeAssertion(((alignment - 1) & (alignment)) == 0);
                OutOfRangeAssertion(alignment <= 128);

                // this alignment is for the end of the allocation
                const size_t requestedMemory = Align<size_t>(sizeof(T) * n, BACK_ALIGNMENT);
                constexpr size_t headSize = sizeof(AllocMetaData);

                for(FreeListItem * iter = freeList.Head(); iter != nullptr; iter = iter->next) {
                    size_t alignmentCorrection = GetAlignmentCorrection(iter->AddressOf(), headSize, alignment);
                    size_t totalSizeInBytes = iter->GetTotalAllocationSize(headSize, requestedMemory, alignment, alignmentCorrection);

                    if(iter->CanFit(totalSizeInBytes)) {
                        freeList.Remove(iter);

                        UndefinedBehaviourAssertion(iter->numBytes >= totalSizeInBytes);

                        uint8_t * pData = reinterpret_cast<uint8_t *>(iter->AddressOf());
                        size_t remainingSpace = iter->numBytes - totalSizeInBytes;

                        // assertion: iter->numBytes - remainingSpace > sizeof(FreeListItem) or == 0

                        if(remainingSpace != 0) {
                            FreeListItem * relocated = Relocate(iter, totalSizeInBytes);
                            relocated->numBytes = remainingSpace;
                            freeList.Insert(relocated);
                        } else {
                            Destroy(iter);
                        }

                        // do not use iter from here on out
                        iter = nullptr;

                        pData += alignmentCorrection;
                        AllocMetaData * metaData = MemoryBlock{ pData }.Construct<AllocMetaData>();

                        metaData->correction = static_cast<uint8_t>(alignmentCorrection);
                        metaData->numElements = n;

                        return MemoryBlock{ reinterpret_cast<void *>(metaData + 1) };
                    }
                }

                uint8_t * const pCurr = header.data + header.offset;

                const size_t alignmentCorrection = GetAlignmentCorrection(pCurr, headSize, alignment);
                const size_t totalSize = alignmentCorrection + headSize + requestedMemory;

                if(HasEnoughPristineSpace(totalSize)) {
                    AllocMetaData * mData = MemoryBlock{ pCurr + alignmentCorrection }.Construct<AllocMetaData>();
                    mData->correction = static_cast<uint8_t>(alignmentCorrection);
                    mData->numElements = n;

                    header.offset += totalSize;

                    return MemoryBlock{ static_cast<void *>(mData + 1) };
                }

                // sign failure
                return MemoryBlock{ nullptr };
            }
        };

        class Resource {
        public:
            size_t defaultPageSize;
            size_t defaultAlignment;

            std::vector<std::unique_ptr<Page, void(*)(void *)>> pages;

            Page * CreatePage(size_t numBytes, size_t alignment) {
                void * p = AlignedMalloc(numBytes + sizeof(Page), alignment);

                OutOfMemoryAssertion(p != nullptr);

                uint8_t * pData = reinterpret_cast<uint8_t *>(p) + sizeof(Page);

                Page * pPage = new (p) Page{ pData, numBytes };

                std::unique_ptr<Page, void(*)(void *)> sPtr{
                    pPage,
                    &AlignedFree
                };

                pages.emplace_back(std::move(sPtr));

                return pPage;
            }

            Resource(size_t defaultPageSize, size_t defaultAlignment) : defaultPageSize{ defaultPageSize }, defaultAlignment{ defaultAlignment } {
                
            }
        };

        std::shared_ptr<Resource> resource;

    public:

        using AllocType = ObjectAllocator;

        template<typename V>
        using DeleterType = AllocatorDeleter<V, AllocType>;

        void ReserveFirstPage() {
            if(resource->pages.empty()) {
                resource->CreatePage(resource->defaultPageSize, resource->defaultAlignment);
            }
        }

        bool operator==(const ObjectAllocator & rhs) const {
            return resource.get() == rhs.resource.get();
        }

        void SetDefaultPageSize(size_t size) {
            resource->defaultPageSize = size;
        }

        void SetDefaultAlignment(size_t alignment) {
            resource->defaultAlignment = alignment;
        }

        ObjectAllocator(size_t defaultPageSize = 65536, size_t defaultAlignment = sizeof(void*)) : resource{ nullptr } {
            resource = std::make_shared<Resource>(defaultPageSize, defaultAlignment);
        }

        ObjectAllocator(const ObjectAllocator &) = default;

        ObjectAllocator(ObjectAllocator &&) noexcept = delete;

        template<typename T>
        MemoryBlock Allocate(size_t numElements) {
            for(auto & i : resource->pages) {
                MemoryBlock b = i->Allocate<T>(numElements, resource->defaultAlignment);

                if(b != nullptr) {
                    return b;
                }
            }

            size_t approxSize = numElements * sizeof(T) + resource->defaultAlignment;

            MemoryBlock blk = resource->CreatePage(std::max(approxSize, resource->defaultPageSize), resource->defaultAlignment)->Allocate<T>(numElements, resource->defaultAlignment);

            UndefinedBehaviourAssertion(blk != nullptr);

            return blk;
        }

        template<typename T>
        void Deallocate(T * ptr, size_t n) {
            for(auto & i : resource->pages) {
                if(i->Owns(ptr)) {
                    i->Deallocate(ptr, n);
                    return;
                }
            }

            UndefinedBehaviourAssertion(false);
        }

        template<typename T, typename ... U>
        std::shared_ptr<T> MakeShared(U && ... args) {
           return std::allocate_shared<T>(StdAllocatorAdapter<T, ObjectAllocator>{ *this }, std::forward<U>(args)...);
        }

        void Defragment(uint64_t maxSteps = std::numeric_limits<uint64_t>::max()) {
            for(auto & i : resource->pages) {
                maxSteps -= i->Defragment(maxSteps);

                if(maxSteps == 0) {
                    return;
                }
            }
        }
    };

}
