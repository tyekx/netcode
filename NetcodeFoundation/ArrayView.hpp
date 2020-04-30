#pragma once

namespace Netcode {
	template<typename T>
	class ArrayView {
		T * beginPtr;
		size_t len;
	public:
		ArrayView(T * ptr, size_t arrayLength) : beginPtr{ ptr }, len{ arrayLength } { }

		ArrayView() = default;
		ArrayView(ArrayView &&) = default;
		ArrayView(const ArrayView &) = default;
		~ArrayView() = default;

		ArrayView & operator=(const ArrayView &) = default;
		ArrayView & operator=(ArrayView &&) = default;

		const T & operator[](size_t idx) const {
			return beginPtr[idx];
		}

		T & operator[](size_t idx) {
			return beginPtr[idx];
		}

		T * begin() {
			return beginPtr;
		}

		T * end() {
			return beginPtr + len;
		}

		size_t Size() const {
			return len;
		}

		T * Data() const {
			return beginPtr;
		}
	};

}
