#include "Allocation.h"
#include <memory>

namespace Egg {

	Allocation::Allocation() noexcept : pointer{ nullptr }, allocationLength{ 0 } { }

	Allocation::Allocation(Allocation && o) noexcept : Allocation() {
		std::swap(pointer, o.pointer);
		std::swap(allocationLength, o.allocationLength);
	}

	Allocation & Allocation::operator=(Allocation o) noexcept {
		std::swap(pointer, o.pointer);
		std::swap(allocationLength, o.allocationLength);
		return *this;
	}

	unsigned int Allocation::SizeInBytes() const {
		return allocationLength;
	}

	void * Allocation::Get() const {
		return pointer;
	}
	 
	bool Allocation::operator!=(std::nullptr_t) const noexcept {
		return pointer != nullptr;
	}

	bool Allocation::operator==(std::nullptr_t) const noexcept {
		return pointer == nullptr;
	}

}
