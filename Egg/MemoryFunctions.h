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
			// @TODO: debug_assert
			return begin[idx];
		}

		T & operator[](size_t idx) {
			return begin[idx];
		}

		size_t Size() const {
			return len;
		}

		T * Data() const {
			return ptr;
		}
	};

	void InterpretSkip(void ** ptr, unsigned int nBytes);

	template<typename T>
	T InterpretAs(void ** ptr) {
		void * src = *ptr;
		T data = *(reinterpret_cast<T *>(src));
		*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(src) + sizeof(T));
		return data;
	}

	template<typename T>
	T * InterpretAsArray(void ** ptr, int len) {
		void * src = *ptr;
		T * data = (reinterpret_cast<T *>(src));
		*ptr = reinterpret_cast<void *>(reinterpret_cast<unsigned char *>(src) + len * sizeof(T));
		return data;
	}

}
