#pragma once

#include <ppl.h>
#include <ppltasks.h>
#include <agents.h>

namespace Netcode {

	template<typename T>
	using Task = concurrency::task<T>;

	template<typename T>
	struct ConcurrentDelete {
		void operator()(T * ptr) const noexcept {
			ptr->~T();
			concurrency::Free(ptr);
		}
	};
	
	template<typename T>
	using ConcurrentPtr = std::unique_ptr<T, ConcurrentDelete<T>>;

	template<typename T, typename ... U>
	ConcurrentPtr<T> MakeConcurrent(U &&... args) {
		void * pData = concurrency::Alloc(sizeof(T));
		return ConcurrentPtr<T>{
			new (pData) T{ std::forward<U>(args)... }
		};
	}

	template<typename T>
	class ConcurrentAllocator {
	public:
		using value_type = T;


		ConcurrentAllocator() = default;
		template <class U> constexpr ConcurrentAllocator(const ConcurrentAllocator<U> &) noexcept {

		}

		[[nodiscard]]
		static T * allocate(size_t n) {
			if(n > (std::numeric_limits<size_t>::max() / sizeof(T))) {
				throw std::bad_alloc{};
			}

			T * p = static_cast<T *>(concurrency::Alloc(n * sizeof(T)));

			if(p != nullptr) {
				return p;
			}

			throw std::bad_alloc{};
		}

		static void deallocate(T * p, size_t n) noexcept {
			concurrency::Free(p);
		}
	};

	template<typename T, typename ... U>
	Ref<T> MakeSharedConcurrent(U && ... args) {
		return std::allocate_shared<T, ConcurrentAllocator<T>>(ConcurrentAllocator<T>{}, std::forward<U>(args)...);
	}
	
	
}
