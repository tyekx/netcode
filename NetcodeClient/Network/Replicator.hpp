#pragma once

#include <NetcodeFoundation/Math.h>
#include <Netcode/HandleDecl.h>
#include "../GameObject.h"

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

		dst = ntohl(*reinterpret_cast<const uint32_t *>(src.Size()));
		return sizeof(uint32_t);
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

		const uint32_t projectedSize = *reinterpret_cast<const uint32_t *>(src.Data());
		const uint32_t readableSize = std::min(projectedSize, static_cast<uint32_t>(src.Size() - sizeof(uint32_t)));
		return readableSize;
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

		return sizeof(uint32_t) + readableLength;
	}
};

