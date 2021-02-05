#pragma once

#include <NetcodeFoundation/Math.h>
#include "../GameObject.h"
#include "NetwDecl.h"

/**
 * Replicator concept
 * @note The user must specify 4 functions:
 *  - static uint32_t Replicate(MutableArrayView<uint8_t> dst, const T & src);
 *  - static uint32_t Replicate(T & dst, ArrayView<uint8_t> src);
 *  - static uint32_t GetReplicatedSize(const T & src);
 *  - static uint32_t QueryReplicatedSize(ArrayView<uint8_t> src);
 */
template<typename>
struct Replicator;

template<>
struct Replicator<uint32_t> {
	constexpr static uint32_t GetReplicatedSize(const uint32_t &) {
		return sizeof(uint32_t);
	}
	
	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		return (src.Size() >= sizeof(uint32_t)) ? sizeof(uint32_t) : 0;
	}
	
	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const uint32_t & src) {
		const uint32_t replSize = GetReplicatedSize(src);
		
		if(dst.Size() < replSize) {
			return 0;
		}

		*reinterpret_cast<uint32_t *>(dst.Data()) = htonl(src);
		return replSize;
	}

	static uint32_t Replicate(uint32_t & dst, Netcode::ArrayView<uint8_t> src) {
		if(src.Size() < sizeof(uint32_t)) {
			return 0;
		}

		dst = ntohl(*reinterpret_cast<const uint32_t *>(src.Data()));
		return sizeof(uint32_t);
	}
};

template<>
struct Replicator<int32_t> {
	constexpr static uint32_t GetReplicatedSize(const int32_t &) {
		return sizeof(int32_t);
	}

	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		return (src.Size() >= sizeof(int32_t)) ? sizeof(int32_t) : 0;
	}

	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const int32_t & src) {
		const uint32_t replSize = GetReplicatedSize(src);

		if(dst.Size() < replSize) {
			return 0;
		}

		*reinterpret_cast<uint32_t *>(dst.Data()) = htonl(static_cast<uint32_t>(src));
		return replSize;
	}

	static uint32_t Replicate(int32_t & dst, Netcode::ArrayView<uint8_t> src) {
		if(src.Size() < sizeof(int32_t)) {
			return 0;
		}

		dst = static_cast<int32_t>(ntohl(*reinterpret_cast<const uint32_t *>(src.Data())));
		return sizeof(int32_t);
	}
};

template<>
struct Replicator<float> {
	constexpr static uint32_t GetReplicatedSize(const float &) {
		return sizeof(float);
	}

	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		return (src.Size() >= sizeof(float)) ? sizeof(float) : 0;
	}
	
	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const float & src) {
		const uint32_t replSize = GetReplicatedSize(src);
		
		if(dst.Size() < replSize) {
			return 0;
		}
		
		static_assert(sizeof(uint32_t) == sizeof(float));
		*reinterpret_cast<uint32_t *>(dst.Data()) = htonf(src);
		return replSize;
	}

	static uint32_t Replicate(float & dst, Netcode::ArrayView<uint8_t> src) {
		if(src.Size() < sizeof(float)) {
			return 0;
		}
		
		static_assert(sizeof(uint32_t) == sizeof(float));
		dst = ntohf(*reinterpret_cast<const uint32_t *>(src.Data()));
		return sizeof(float);
	}
};

template<>
struct Replicator<Netcode::Float2> {
	constexpr static uint32_t GetReplicatedSize(const Netcode::Float2 &) {
		return 2 * sizeof(float);
	}

	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		constexpr uint32_t sz = 2 * sizeof(float);
		return (src.Size() >= sz) ? sz : 0;
	}
	
	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const Netcode::Float2 & src) {
		return
			Replicator<float>::Replicate(dst, src.x) +
			Replicator<float>::Replicate(dst.Offset(4), src.y);
	}

	static uint32_t Replicate(Netcode::Float2 & dst, Netcode::ArrayView<uint8_t> src) {
		return
			Replicator<float>::Replicate(dst.x, src) +
			Replicator<float>::Replicate(dst.y, src.Offset(4));
	}
};

template<>
struct Replicator<Netcode::Float3> {
	constexpr static uint32_t GetReplicatedSize(const Netcode::Float3 &) {
		return 3 * sizeof(float);
	}

	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		constexpr uint32_t sz = 3 * sizeof(float);
		return (src.Size() >= sz) ? sz : 0;
	}
	
	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const Netcode::Float3 & src) {
		return
			Replicator<float>::Replicate(dst, src.x) +
			Replicator<float>::Replicate(dst.Offset(4), src.y) +
			Replicator<float>::Replicate(dst.Offset(8), src.z);
	}

	static uint32_t Replicate(Netcode::Float3 & dst, Netcode::ArrayView<uint8_t> src) {
		return 
			Replicator<float>::Replicate(dst.x, src) +
			Replicator<float>::Replicate(dst.y, src.Offset(4)) + 
			Replicator<float>::Replicate(dst.z, src.Offset(8));
	}
};

template<>
struct Replicator<std::string> {
	static uint32_t GetReplicatedSize(const std::string & str) {
		return sizeof(uint32_t) + str.size();
	}

	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		if(src.Size() < sizeof(uint32_t)) {
			return 0;
		}

		const uint32_t projectedSize = ntohl(*reinterpret_cast<const uint32_t *>(src.Data()));
		const uint32_t readableSize = std::min(projectedSize, static_cast<uint32_t>(src.Size() - sizeof(uint32_t)));
		return readableSize + sizeof(uint32_t);
	}
	
	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const std::string & src) {
		const uint32_t replSize = GetReplicatedSize(src);
		
		if(dst.Size() < replSize) {
			return 0;
		}

		Replicator<uint32_t>::Replicate(dst, static_cast<uint32_t>(src.size()));
		dst = dst.Offset(4);

		memcpy(dst.Data(), src.data(), src.size());

		return replSize;
	}
	
	static uint32_t Replicate(std::string & dst, Netcode::ArrayView<uint8_t> src) {
		if(src.Size() < sizeof(uint32_t)) {
			return 0;
		}

		uint32_t numChars = 0;
		const uint32_t replicatedBytes = Replicator<uint32_t>::Replicate(numChars, src);

		if(replicatedBytes != sizeof(uint32_t)) {
			return 0;
		}

		src = src.Offset(sizeof(uint32_t));

		uint32_t readableLength = std::min(numChars, static_cast<uint32_t>(src.Size()));

		std::string str;
		str.assign(reinterpret_cast<const char *>(src.Data()), readableLength);

		dst = std::move(str);

		return sizeof(uint32_t) + readableLength;
	}
};

namespace Detail {

	template<typename T>
	struct CompositeReplicateImpl {
		template<typename HEAD, typename ... TAIL>
		static uint32_t Run(Netcode::MutableArrayView<uint8_t> dst, const T & v, HEAD T:: * mainPtr, TAIL ... ptrs) {
			const uint32_t value = Replicator<HEAD>::Replicate(dst, v.*mainPtr);

			if constexpr (sizeof...(TAIL) == 0) {
				return value;
			} else {
				dst = dst.Offset(value);
				
				const uint32_t tailSum = Run(dst, v, ptrs...);

				if(tailSum == 0 || value == 0)
					return 0;

				return value + tailSum;
			}
		}
		
		template<typename HEAD, typename ... TAIL>
		static uint32_t Run(T & v, Netcode::ArrayView<uint8_t> src, HEAD T:: * mainPtr, TAIL ... ptrs) {
			const uint32_t value = Replicator<HEAD>::Replicate(v.*mainPtr, src);

			if constexpr(sizeof...(TAIL) == 0) {
				return value;
			} else {
				src = src.Offset(value);
				
				const uint32_t tailSum = Run(v, src, ptrs...);

				if(tailSum == 0 || value == 0)
					return 0;

				return value + tailSum;
			}
		}
	};

	template<typename T, typename ... U>
	uint32_t CompositeReplicate(Netcode::MutableArrayView<uint8_t> dst, const T & value, U T:: * ... memptr) {
		return CompositeReplicateImpl<T>::Run(dst, value, memptr...);
	}

	template<typename T, typename ... U>
	uint32_t CompositeReplicate(T & value, Netcode::ArrayView<uint8_t> src, U T:: * ... memptr) {
		return CompositeReplicateImpl<T>::Run(value, src, memptr...);
	}

	template<typename T>
	struct CompositeGetReplicatedSizeImpl {
		static uint32_t Run(const T & v) { return 0; }

		template<typename HEAD, typename ... TAIL>
		static uint32_t Run(const T & v, HEAD T:: * mainPtr, TAIL ... ptrs) {
			uint32_t value = Replicator<HEAD>::GetReplicatedSize(v.*mainPtr);
			return value + Run(v, ptrs...);
		}
	};

	template<typename T, typename ... U>
	uint32_t CompositeGetReplicatedSize(const T & value, U T:: * ... memptr) {
		return CompositeGetReplicatedSizeImpl<T>::Run(value, memptr...);
	}
	
	template<typename HEAD, typename ... TAIL>
	struct CompositeQueryReplicatedSizeImpl {
		static uint32_t Run(Netcode::ArrayView<uint8_t> src) {
			const uint32_t numBytes = Replicator<HEAD>::QueryReplicatedSize(src);

			if(numBytes == 0) {
				return 0;
			}
			
			if constexpr(sizeof...(TAIL) == 0) {
				return numBytes;
			} else {
				const uint32_t tailSum = CompositeQueryReplicatedSizeImpl<TAIL...>::Run(src.Offset(numBytes));

				if(tailSum == 0)
					return 0;

				return tailSum + numBytes;
			}
		}
	};

	template<typename T, typename ... U>
	uint32_t CompositeQueryReplicatedSize(Netcode::ArrayView<uint8_t> src, U T:: * ...) {
		return CompositeQueryReplicatedSizeImpl<U...>::Run(src);
	}
}

template<>
struct Replicator<PlayerStatEntry> {
	static uint32_t GetReplicatedSize(const PlayerStatEntry& entry) {
		return Detail::CompositeGetReplicatedSize(entry,
			&PlayerStatEntry::name,
			&PlayerStatEntry::id,
			&PlayerStatEntry::deaths,
			&PlayerStatEntry::kills,
			&PlayerStatEntry::ping);
	}

	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		return Detail::CompositeQueryReplicatedSize(src,
			&PlayerStatEntry::name,
			&PlayerStatEntry::id,
			&PlayerStatEntry::deaths,
			&PlayerStatEntry::kills,
			&PlayerStatEntry::ping);
	}

	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const PlayerStatEntry & src) {
		return Detail::CompositeReplicate(dst, src,
			&PlayerStatEntry::name,
			&PlayerStatEntry::id,
			&PlayerStatEntry::deaths,
			&PlayerStatEntry::kills,
			&PlayerStatEntry::ping);
	}

	static uint32_t Replicate(PlayerStatEntry & dst, Netcode::ArrayView<uint8_t> src) {
		return Detail::CompositeReplicate(dst, src,
			&PlayerStatEntry::name,
			&PlayerStatEntry::id,
			&PlayerStatEntry::deaths,
			&PlayerStatEntry::kills,
			&PlayerStatEntry::ping);
	}
};

template<typename T>
struct Replicator<std::vector<T>> {
	static uint32_t GetReplicatedSize(const std::vector<T> & vec) {
		uint32_t sumSize = 0;

		for(const T& item : vec) {
			sumSize += Replicator<T>::GetReplicatedSize(item);
		}

		return sizeof(uint32_t) + sumSize;
	}

	static uint32_t QueryReplicatedSize(Netcode::ArrayView<uint8_t> src) {
		if(src.Size() < sizeof(uint32_t)) {
			return 0;
		}

		const uint32_t projectedSize = ntohl(*reinterpret_cast<const uint32_t *>(src.Data()));

		src = src.Offset(sizeof(uint32_t));
		
		uint32_t sumSize = sizeof(uint32_t);
		for(uint32_t i = 0; i < projectedSize; i++) {
			uint32_t numBytes = Replicator<T>::QueryReplicatedSize(src);

			if(numBytes == 0)
				return 0;
			
			sumSize += numBytes;
			src = src.Offset(numBytes);
		}
		
		return sumSize;
	}

	static uint32_t Replicate(Netcode::MutableArrayView<uint8_t> dst, const std::vector<T> & vec) {
		uint32_t sizeSum = Replicator<uint32_t>::Replicate(dst, static_cast<uint32_t>(vec.size()));

		if(sizeSum == 0)
			return 0;
		
		dst = dst.Offset(sizeSum);

		for(const T & item : vec) {
			uint32_t numBytes = Replicator<T>::Replicate(dst, item);

			if(dst.Size() < numBytes)
				return 0;
			
			dst = dst.Offset(numBytes);
			sizeSum += numBytes;
		}

		return sizeSum;
	}

	static uint32_t Replicate(std::vector<T> & dst, Netcode::ArrayView<uint8_t> src) {
		uint32_t projectedSize;
		uint32_t replicatedBytes = Replicator<uint32_t>::Replicate(projectedSize, src);

		if(replicatedBytes == 0)
			return 0;

		dst.clear();
		dst.reserve(projectedSize);

		src = src.Offset(replicatedBytes);

		for(uint32_t i = 0; i < projectedSize; i++) {
			T value;
			uint32_t numBytes = Replicator<T>::Replicate(value, src);

			if(numBytes == 0) {
				dst.clear();
				return 0;
			}

			src = src.Offset(numBytes);
			replicatedBytes += numBytes;
			dst.push_back(value);
		}

		return replicatedBytes;
	}
};
