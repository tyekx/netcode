#pragma once

#include <Netcode/HandleDecl.h>
#include "NetworkDecl.h"

namespace Netcode::Network {

	struct MtuValue {
		constexpr static uint32_t DEFAULT = 1280;
		constexpr static uint32_t UDP_HEADER_SIZE = 8;
		constexpr static uint32_t IPV4_HEADER_SIZE = 20;
		constexpr static uint32_t IPV6_HEADER_SIZE = 40;
		// record layer header size
		constexpr static uint32_t DTLS_RL_HEADER_SIZE = 13;

	private:
		uint32_t mtu;
	public:
		MtuValue(const MtuValue & rhs) = default;
		MtuValue & operator=(const MtuValue & rhs) = default;

		MtuValue() : mtu{ 0 } { }

		explicit MtuValue(uint32_t v) : mtu{ v } {

		}

		[[nodiscard]]
		uint32_t GetMtu() const {
			return mtu;
		}

		[[nodiscard]]
		uint32_t GetIpPayloadSize(const IpAddress & peerOrHostAddr) const {
			if(peerOrHostAddr.is_v4()) {
				return mtu - IPV4_HEADER_SIZE;
			}

			if(peerOrHostAddr.is_v6()) {
				return mtu - IPV6_HEADER_SIZE;
			}

			return mtu;
		}

		[[nodiscard]]
		uint32_t GetUdpPayloadSize(const IpAddress & peerOrHostAddr) const {
			return GetIpPayloadSize(peerOrHostAddr) - UDP_HEADER_SIZE;
		}

		[[nodiscard]]
		uint32_t GetDtlsPayloadSize(const IpAddress & peerOrHostAddr) const {
			return GetUdpPayloadSize(peerOrHostAddr) - DTLS_RL_HEADER_SIZE;
		}
	};
	
}
