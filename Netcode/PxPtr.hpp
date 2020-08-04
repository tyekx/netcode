#pragma once

#include <utility>

namespace Netcode {

	template<typename T>
	class PxPtr {
		T * ptr;


	public:
		void Reset(T * p = nullptr) {
			if(ptr) {
				ptr->release();
			}

			ptr = p;
		}

		~PxPtr() noexcept {
			try {
				Reset();
			} catch(...) { }
		}

		PxPtr() : ptr{ nullptr } { }

		PxPtr(T * p) : ptr{ p } { }

		PxPtr(PxPtr<T> && rhs) : ptr{ std::move(rhs.ptr) } {
			rhs.ptr = nullptr;
		}

		PxPtr & operator=(PxPtr<T> && rhs) noexcept {
			std::swap(ptr, rhs.ptr);
			return *this;
		}

		PxPtr & operator=(T * p) {
			Reset(p);
			return *this;
		}

		constexpr auto * operator->() {
			return ptr;
		}

		constexpr auto * operator->() const {
			return ptr;
		}

		T ** GetAddressOf() {
			return &ptr;
		}

		T * Get() {
			return ptr;
		}

		T & operator*() {
			return *ptr;
		}

		bool operator!=(std::nullptr_t) const {
			return ptr != nullptr;
		}

		bool operator==(std::nullptr_t) const {
			return ptr == nullptr;
		}

	};

}
