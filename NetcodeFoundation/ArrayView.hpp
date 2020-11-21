#pragma once

#include <algorithm>

namespace Netcode {

	template<typename T>
	class ArrayViewBase {
	protected:
		T * beginPtr;
		size_t length;

	public:
		ArrayViewBase() noexcept = default;
		ArrayViewBase(ArrayViewBase &&) noexcept = default;
		ArrayViewBase(const ArrayViewBase &) noexcept = default;
		~ArrayViewBase() noexcept = default;
		ArrayViewBase(T * ptr, size_t length) noexcept : beginPtr { ptr }, length{ length } { }

		ArrayViewBase<T> & operator=(const ArrayViewBase &) noexcept = default;
		ArrayViewBase<T> & operator=(ArrayViewBase &&) noexcept = default;
	};

	template<typename T>
	class MutableArrayView : public ArrayViewBase<T> {
	public:
		using ArrayViewBase<T>::ArrayViewBase;

		MutableArrayView<T> Offset(size_t numElements) const {
			size_t offset = std::min(this->length, numElements);
			
			return MutableArrayView<T>{
				this->beginPtr + offset,
				this->length - offset
			};
		}

		const T & operator[](size_t idx) const {
			return this->beginPtr[idx];
		}

		const T * cbegin() const {
			return this->beginPtr;
		}

		const T * cend() const {
			return this->beginPtr + this->length;
		}

		T & operator[](size_t idx) {
			return this->beginPtr[idx];
		}

		T * begin() {
			return this->beginPtr;
		}

		T * end() {
			return this->beginPtr + this->length;
		}

		size_t Size() const {
			return this->length;
		}

		T * Data() const {
			return this->beginPtr;
		}
	};

	template<typename T>
	class ArrayView : public ArrayViewBase<const T> {
	public:
		using ArrayViewBase<const T>::ArrayViewBase;

		ArrayView(const MutableArrayView<T> & mutableView) : ArrayViewBase<const T>(mutableView.Data(), mutableView.Size()) { }

		ArrayView<T> Offset(size_t numElements) const {
			size_t offset = std::min(this->length, numElements);
			
			return ArrayView<T>{
				this->beginPtr + offset,
				this->length - offset
			};
		}
		
		const T & operator[](size_t idx) const {
			return this->beginPtr[idx];
		}

		const T * begin() const {
			return this->beginPtr;
		}

		const T * end() const {
			return this->beginPtr + this->length;
		}

		size_t Size() const {
			return this->length;
		}

		const T * Data() const {
			return this->beginPtr;
		}
	};

}
