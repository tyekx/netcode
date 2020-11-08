#pragma once

#include <Netcode/System/TimeTypes.h>
#include <NetcodeFoundation/Macros.h>
#include <boost/asio/buffer.hpp>

namespace Netcode::Network {

	struct FragmentData {
		uint8_t fragmentIndex;
		uint8_t fragmentCount;
		uint16_t sizeInBytes;

		FragmentData() : fragmentIndex{ 0 }, fragmentCount{ 0 }, sizeInBytes{ 0 } { }

		explicit FragmentData(uint8_t fragIdx, uint8_t fragCount, uint16_t sizeInBytes) :
			fragmentIndex{ fragIdx }, fragmentCount{ fragCount }, sizeInBytes{ sizeInBytes } { }

		explicit FragmentData(uint32_t value) {
			Unpack(value);
		}

		void Unpack(uint32_t value) {
			fragmentIndex = static_cast<uint8_t>((value >> 24) & 0xFF);
			fragmentCount = static_cast<uint8_t>((value >> 16) & 0xFF);
			sizeInBytes = static_cast<uint16_t>(value & 0xFFFF);
		}

		uint32_t Pack() const {
			return (static_cast<uint32_t>(fragmentIndex) << 24) |
				(static_cast<uint32_t>(fragmentCount) << 16) |
				static_cast<uint32_t>(sizeInBytes);
		}
	};

	template<typename ProtocolType>
	struct BasicPacket {
		using EndpointType = typename ProtocolType::endpoint;

		EndpointType endpoint;
		uint32_t sequence;
		Timestamp timestamp;
		uint32_t size;
		uint32_t capacity;
		uint8_t * data;

		~BasicPacket() noexcept = default;

		BasicPacket() noexcept : endpoint{}, sequence{ 0 }, timestamp{}, size{}, capacity{}, data{} {

		}

		BasicPacket(uint8_t * pData, uint32_t dataSizeInBytes) noexcept : BasicPacket{} {
			SetDataUnsafe(pData, dataSizeInBytes);
		}

		BasicPacket(const BasicPacket & rhs) noexcept = default;
		BasicPacket & operator=(const BasicPacket & bp) noexcept = default;

		NETCODE_CONSTRUCTORS_DELETE_MOVE(BasicPacket);

		void SetDataUnsafe(uint8_t * pData, uint32_t dataSizeInBytes) noexcept {
			data = pData;
			capacity = dataSizeInBytes;
		}

		[[nodiscard]]
		uint32_t GetSize() const noexcept { return size; }
		void SetSize(uint32_t s) noexcept { size = s; }

		[[nodiscard]]
		uint32_t GetSequence() const noexcept { return sequence; }
		void SetSequence(uint32_t seq) noexcept { sequence = seq; }

		[[nodiscard]]
		Timestamp GetTimestamp() const { return timestamp; }
		void SetTimestamp(Timestamp t) { timestamp = t; }

		[[nodiscard]]
		const EndpointType & GetEndpoint() const noexcept { return endpoint; }
		
		[[nodiscard]]
		EndpointType & GetEndpoint() noexcept { return endpoint; }
		void SetEndpoint(const EndpointType & ep) noexcept { endpoint = ep; }

		[[nodiscard]]
		const uint8_t * GetData() const noexcept { return data; }
		[[nodiscard]]
		uint8_t * GetData() noexcept { return data; }

		[[nodiscard]]
		uint32_t GetCapacity() const noexcept { return capacity; }

		[[nodiscard]] // [ data, capacity ]: the entire buffer is writable
		boost::asio::mutable_buffer GetMutableBuffer() noexcept {
			return boost::asio::mutable_buffer{ GetData(), GetCapacity() };
		}

		[[nodiscard]] // [ data, size ]: only the written part is readable
		boost::asio::const_buffer GetConstBuffer() const noexcept {
			return boost::asio::const_buffer{ GetData(), GetSize() };
		}

	};

	struct UdpPacket : public BasicPacket<boost::asio::ip::udp> {
		constexpr static uint32_t HEADER_SIZE = 8;
		constexpr static uint32_t MAX_SIZE = 65535 - HEADER_SIZE;
		constexpr static uint32_t IPV4_MAX_SIZE = 65535 - HEADER_SIZE - 20;
		constexpr static uint32_t IPV6_MAX_SIZE = 65535 - HEADER_SIZE - 40;

		using BasicPacket::BasicPacket;
	};

}
