#pragma once

namespace Egg {
	template<typename T>
	class ArrayView {
		T * begin;
		size_t len;
	public:
		ArrayView(T * ptr, size_t arrayLength) : begin{ ptr }, len{ arrayLength } { }

		ArrayView() = default;
		ArrayView(ArrayView &&) = default;
		ArrayView(const ArrayView &) = default;
		~ArrayView() = default;

		ArrayView & operator=(const ArrayView &) = default;
		ArrayView & operator=(ArrayView &&) = default;

		const T & operator[](size_t idx) const {
			return begin[idx];
		}

		T & operator[](size_t idx) {
			return begin[idx];
		}

		size_t Size() const {
			return len;
		}

		T * Data() const {
			return begin;
		}
	};

}
