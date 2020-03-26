#pragma once

#include <Egg/EggMpl.hpp>
#include <cstdint>

template<typename TUPLE_T, typename TUPLE_T2>
struct TupleMoveStorageImpl;

template<typename TUPLE_T>
struct TupleMoveStorageImpl<TUPLE_T, std::tuple<>> {
	static void Invoke(uint8_t * dst, uint8_t * src, SignatureType sig) {

	}
};

template<typename TUPLE_T, typename HEAD, typename ... TAIL>
struct TupleMoveStorageImpl<TUPLE_T, std::tuple<HEAD, TAIL...>> {
	static void Invoke(uint8_t * dst, uint8_t * src, SignatureType sig) {
		using ComponentType = HEAD;
		constexpr static uint32_t offsetOf = TupleOffsetOf<ComponentType, TUPLE_T>::value;
		constexpr static SignatureType mask = TupleCreateMask<std::tuple<ComponentType>, TUPLE_T>::value;

		if((sig & mask) != 0ull) {
			ComponentType * lhs = reinterpret_cast<ComponentType *>(dst + offsetOf);
			ComponentType * rhs = reinterpret_cast<ComponentType *>(src + offsetOf);
			std::swap(*lhs, *rhs);
		}

		TupleMoveStorageImpl<TUPLE_T, std::tuple<TAIL...>>::Invoke(dst, src, sig);
	}
};

template<typename T>
struct TupleMoveStorage {
	static void Invoke(uint8_t * dst, uint8_t * src, SignatureType signature) {
		TupleMoveStorageImpl< T, T >::Invoke(dst, src, signature);
	}
};

template<typename COMPONENTS_T, typename EXTENSION_COMPONENTS_T>
class ComponentStorage {
public:
	using ALL_COMPONENTS_T = typename TupleMerge<COMPONENTS_T, EXTENSION_COMPONENTS_T>::type;

private:
	uint8_t storage[TupleSizeofSum<COMPONENTS_T>::value];
	uint8_t * extendedStorage;

	void InitExtendedStorage() {
		if(extendedStorage == nullptr) {
			extendedStorage = reinterpret_cast<uint8_t *>(std::malloc(TupleSizeofSum<EXTENSION_COMPONENTS_T>::value));
		}
	}

	template<typename T>
	bool HasExtensionComponent() {
		constexpr SignatureType componentIdx = TupleCountOf<COMPONENTS_T>::value + TupleIndexOf<T, EXTENSION_COMPONENTS_T>::value;
		return (signature & (1ULL << componentIdx)) > 0;
	}

	template<typename T>
	T * GetExtensionComponent() {
		return reinterpret_cast<T *>(extendedStorage + TupleOffsetOf<T, EXTENSION_COMPONENTS_T>::value);
	}

	template<typename T>
	T * AddExtensionComponent() {
		if(!HasExtensionComponent<T>()) {
			InitExtendedStorage();
			constexpr SignatureType componentIdx = TupleCountOf<COMPONENTS_T>::value + TupleIndexOf<T, EXTENSION_COMPONENTS_T>::value;
			T * ptr = GetExtensionComponent<T>();
			signature |= (1ULL << componentIdx);
			new (ptr) T();
			return ptr;
		}
		return GetExtensionComponent<T>();
	}

	template<typename T>
	void RemoveExtensionComponent() {
		if(HasExtensionComponent<T>()) {
			constexpr SignatureType componentIdx = TupleCountOf<COMPONENTS_T>::value + TupleIndexOf<T, EXTENSION_COMPONENTS_T>::value;
			T * ptr = GetExtensionComponent<T>();
			ptr->~T();
			signature &= (~(1ULL << componentIdx));
		}
	}

public:
	SignatureType signature;

	ComponentStorage() = default;

	~ComponentStorage() noexcept {
		CompositeObjectDestructor<COMPONENTS_T>::Invoke(signature, storage);
		if(extendedStorage != nullptr) {
			signature >>= TupleCountOf<COMPONENTS_T>::value;
			CompositeObjectDestructor<EXTENSION_COMPONENTS_T>::Invoke(signature, extendedStorage);
			std::free(extendedStorage);
			extendedStorage = nullptr;
		}
	}

	ComponentStorage & operator=(ComponentStorage rhs) noexcept {
		constexpr static uint32_t StorageSize = TupleSizeofSum<COMPONENTS_T>::value;
		uint8_t tempStorage[StorageSize];

		SignatureType signatureMask = TupleMaxMaskValue<COMPONENTS_T>::value;
		SignatureType mainStorageSignature = signature & signatureMask;
		SignatureType rhsMainStorageSignature = rhs.signature & signatureMask;

		TupleMoveStorage<COMPONENTS_T>::Invoke(tempStorage, storage, mainStorageSignature);
		TupleMoveStorage<COMPONENTS_T>::Invoke(storage, rhs.storage, rhsMainStorageSignature);
		TupleMoveStorage<COMPONENTS_T>::Invoke(rhs.storage, tempStorage, mainStorageSignature);

		std::swap(signature, rhs.signature);
		std::swap(extendedStorage, rhs.extendedStorage);
		return *this;
	}

	template<typename T>
	bool HasComponent() {
		static_assert(TupleContainsType<T, ALL_COMPONENTS_T>::value, "Component type was not found");

		if constexpr(TupleContainsType<T, COMPONENTS_T>::value) {
			return (signature & (1ULL << TupleIndexOf<T, COMPONENTS_T>::value)) > 0;
		} else {
			return HasExtensionComponent<T>();
		}
	}

	template<typename T>
	T * GetComponent() {
		static_assert(TupleContainsType<T, ALL_COMPONENTS_T>::value, "Component type was not found");

#if _DEBUG
		SignatureType sigMask = 1ull << TupleIndexOf<T, ALL_COMPONENTS_T>::value;
		// force a crash in debug mode
		if((signature & sigMask) == 0) {
			return nullptr;
		}
#endif

		if constexpr(TupleContainsType<T, COMPONENTS_T>::value) {
			return reinterpret_cast<T *>(storage + TupleOffsetOf<T, COMPONENTS_T>::value);
		} else {
			return GetExtensionComponent<T>();
		}
	}

	template<typename T>
	void RemoveComponent() {
		static_assert(TupleContainsType<T, ALL_COMPONENTS_T>::value, "Component type was not found");

		if constexpr(TupleContainsType<T, COMPONENTS_T>::value) {
			if(HasComponent<T>()) {
				T * ptr = GetComponent<T>();
				signature &= (~(1ULL << TupleIndexOf<T, COMPONENTS_T>::value));
				ptr->~T();
			}
		} else {
			return RemoveExtensionComponent<T>();
		}
	}

	template<typename T>
	T * AddComponent() {
		static_assert(TupleContainsType<T, ALL_COMPONENTS_T>::value, "Component type was not found");

		if constexpr(TupleContainsType<T, COMPONENTS_T>::value) {
			if(!HasComponent<T>()) {
				signature |= (1ULL << TupleIndexOf<T, COMPONENTS_T>::value);
				T * ptr = GetComponent<T>();
				new (ptr) T();
				return ptr;
			}
			return GetComponent<T>();
		} else {
			return AddExtensionComponent<T>();
		}
	}
};
