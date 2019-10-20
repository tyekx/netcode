#pragma once

namespace Egg {

	/*
	* For later use, represents memory a memory allocation
	*/
	class Allocation {
		void * pointer;
		unsigned int allocationLength;
	public:
		Allocation() noexcept;

		~Allocation() noexcept = default;

		Allocation(Allocation && o) noexcept;

		Allocation & operator=(Allocation o) noexcept;

		unsigned int SizeInBytes() const;

		void * Get() const;

		bool operator!=(std::nullptr_t) const noexcept;

		bool operator==(std::nullptr_t) const noexcept;

		template<typename T>
		T * As() {
			return static_cast<T *>(pointer);
		}
	};

}
