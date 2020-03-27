#pragma once

#include <type_traits>
#include "Allocator.hpp"

namespace Egg {

	template<typename T>
	class SharedPtr {

		struct ControlBlock {
			T * source;
			int instances;
		};

		ControlBlock * controlBlock;

	public:
		
		SharedPtr(Allocator & alloc) : controlBlock{ nullptr } {
			static_assert(!std::is_array<T>::value, "Array types are not yet supported");
		}


	};

}
