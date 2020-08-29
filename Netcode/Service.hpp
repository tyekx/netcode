#pragma once

#include "NetcodeMpl.hpp"

namespace Netcode {

	/**
	* A simple dependency injector for the user, so that any dependency can be initialized, fetched, destroyed on demand
	*/
	template<typename TUPLE_T>
	class Service {

		static SignatureType signature;
		static uint8_t data[TupleSizeofSum<TUPLE_T>::value];

	public:

		static void Clear() {
			CompositeObjectDestructor<TUPLE_T>::Invoke(signature, data);
		}

		template<typename T, typename ... U>
		static void Init(U && ... args) {
			static_assert(TupleContainsType<T, TUPLE_T>::value, "Can not initialize a service that is not declared up front");

			constexpr uint32_t offsetOf = TupleOffsetOf<T, TUPLE_T>::value;
			constexpr uint32_t indexOf = TupleIndexOf<T, TUPLE_T>::value;
			constexpr SignatureType mask = 1ull << indexOf;

			if((signature & mask) == 0) {
				signature |= mask;
				new (reinterpret_cast<T *>(data + offsetOf)) T(std::forward<U>(args)...);
			}
			// else debug assert
		}

		template<typename T>
		static typename std::remove_pointer<T>::type * Get() {
			static_assert(TupleContainsType<T, TUPLE_T>::value, "Can not initialize a service that is not declared up front");

			constexpr uint32_t offsetOf = TupleOffsetOf<T, TUPLE_T>::value;

			if constexpr(std::is_pointer<T>::value) {
				return *reinterpret_cast<T *>(data + offsetOf);
			} else {
				return reinterpret_cast<T *>(data + offsetOf);
			}
		}

	};

	template<typename TUPLE_T>
	SignatureType Service<TUPLE_T>::signature{};

	template<typename TUPLE_T>
	uint8_t Service<TUPLE_T>::data[TupleSizeofSum<TUPLE_T>::value] = {};

}
