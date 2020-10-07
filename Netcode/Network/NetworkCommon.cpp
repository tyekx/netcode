#include <algorithm>

#include "NetworkCommon.h"
#include "../Logger.h"
#include "Macros.h"
#include <NetcodeFoundation/Exceptions.h>
#include <string>
#include <iphlpapi.h>
#include <WinSock2.h>


namespace Netcode::Network {

	NetworkContext::~NetworkContext() {
		Stop();
	}

	NetworkContext::NetworkContext() : ioc{}, work{ std::make_unique<boost::asio::io_context::work>(ioc) }, workers{} {
	
	}

	void NetworkContext::Start(uint8_t numThreads) {
		if(!workers.empty()) {
			return;
		}

		numThreads = std::clamp(numThreads, static_cast<uint8_t>(1), static_cast<uint8_t>(4));

		workers.reserve(numThreads);

		for(uint8_t i = 0; i < numThreads; ++i) {
			workers.emplace_back([this]() -> void {
				ioc.run();
			});
		}
	}

	void NetworkContext::Stop() {
		if(workers.empty()) {
			return;
		}

		if(work) {
			work.reset();
		}

		int32_t numThreads = static_cast<int32_t>(workers.size());

		ioc.stop();

		for(auto & thread : workers) {
			thread.join();
		}

		workers.clear();

		Log::Info("[Network] ({0}) I/O threads were joined successfully", numThreads);
	}

	boost::asio::io_context & NetworkContext::GetImpl() {
		return ioc;
	}

	ErrorCode Bind(const boost::asio::ip::address & selfAddr, UdpSocket & udpSocket, uint32_t & port) {
		uint32_t portHint = port;
		port = std::numeric_limits<uint32_t>::max();

		if(portHint < 1024 || portHint > 49151) {
			portHint = (49151 - 1024) / 2;
		}

		ErrorCode ec;
		const uint32_t range = 49151 - 1024;
		const int32_t start = static_cast<int32_t>(portHint);
		int32_t sign = 1;

		UdpEndpoint endpoint{ selfAddr, 0 };

		udpSocket.open(endpoint.protocol(), ec);

		if(ec) {
			return ec;
		}

		for(int32_t i = 0; i < range; ++i, sign = -sign) {
			uint32_t portToTest = static_cast<uint32_t>(start + sign * i / 2);

			if(portToTest < 1024 || portToTest > 49151) {
				continue;
			}

			endpoint.port(portToTest);

			udpSocket.bind(endpoint, ec);

			if(ec == boost::asio::error::bad_descriptor) { // expected error
				continue;
			}

			if(ec) { // unexpected error 
				return ec;
			}

			port = portToTest;

			return Errc::make_error_code(Errc::success);
		}

		return ec;
	}
	
	static IpAddress ConvertToAsioAddress(SOCKET_ADDRESS addr) {
		/*
		 * copying bytes would be faster but this is ABI agnostic
		 */
		wchar_t buffer[64];
		char narrowBuffer[64];
		DWORD s = 64;
		WSAAddressToStringW(addr.lpSockaddr, addr.iSockaddrLength, nullptr, buffer, &s);
		
		int dstSize = WideCharToMultiByte(CP_UTF8, 0, buffer, s, narrowBuffer, 64, nullptr, nullptr);
		
		return boost::asio::ip::make_address(std::string_view{ narrowBuffer, static_cast<size_t>(dstSize) });
	}

	static std::vector<Interface> GetNativeNetworkInterfaces(DWORD addrFamily) {
		ULONG workingBufferSize = 65536;
		std::unique_ptr<BYTE[]> workingBuffer = std::make_unique<BYTE[]>(workingBufferSize);
		std::vector<Interface> ifaces;
		ifaces.reserve(4);

		IP_ADAPTER_ADDRESSES * currentAdapter = reinterpret_cast<IP_ADAPTER_ADDRESSES *>(workingBuffer.get());
		
		ULONG status = GetAdaptersAddresses(addrFamily, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST, nullptr, currentAdapter, &workingBufferSize);

		if(status == ERROR_BUFFER_OVERFLOW) {
			throw OutOfRangeException{ "Query buffer is insufficient" };
		}

		if(status != NO_ERROR) {
			return std::vector<Interface>{};
		}

		while(currentAdapter != nullptr) {
			if(currentAdapter->FirstUnicastAddress == nullptr) {
				currentAdapter = currentAdapter->Next;
				continue;
			}
			
			Interface neti;
			
			neti.address = ConvertToAsioAddress(currentAdapter->FirstUnicastAddress->Address);
			neti.description = currentAdapter->Description;
			neti.friendlyName = currentAdapter->FriendlyName;
			neti.uplinkSpeedBps = currentAdapter->TransmitLinkSpeed;
			neti.downlinkSpeedBps = currentAdapter->ReceiveLinkSpeed;
			neti.mtu = currentAdapter->Mtu;

			/*
			 * Rank should be non zero if IPv4, IPv6 is enabled and its operating
			 */
			ULONG rank = currentAdapter->Ipv6Enabled * currentAdapter->Ipv4Enabled * (!currentAdapter->ReceiveOnly)
				* (currentAdapter->OperStatus == IfOperStatusUp);

			switch(currentAdapter->IfType) {
				case IF_TYPE_ETHERNET_CSMACD:
					neti.netcodeRank = 10 * rank;
					break;
				case IF_TYPE_IEEE80211:
					neti.netcodeRank = 5 * rank;
					break;
				default: {
					/*
					 * not the most common WIFI/ethernet case, try it from the DHCP direction
					 */
					IP_PREFIX_ORIGIN prefixOrigin = currentAdapter->FirstUnicastAddress->PrefixOrigin;


					switch(prefixOrigin) {
						// DHCP is a good candidate
						case IpPrefixOriginDhcp: neti.netcodeRank = 3 * rank; break;

						// manual could be an okay one
						case IpPrefixOriginManual: neti.netcodeRank = 2 * rank; break;
						
						// meh candidates
						default: neti.netcodeRank = rank; break;
					}
				} break;
			}

			ifaces.emplace_back(std::move(neti));

			currentAdapter = currentAdapter->Next;
		}

		// order by netcode rank
		std::sort(std::begin(ifaces), std::end(ifaces), [](const Interface & a, const Interface & b) -> bool {
			return a.netcodeRank > b.netcodeRank;
		});

		return ifaces;
	}

	bool SetDontFragmentBit(UdpSocket & socket) {
		if(!socket.is_open()) {
			return false;
		}
		
		
		SOCKET s = socket.native_handle();

		int value = 1;
		int status = -1;

		ErrorCode ec;
		auto endpoint = socket.local_endpoint(ec);

		if(ec) {
			return false;
		}

		auto addr = endpoint.address();
		
		if(addr.is_v4()) {
			status = setsockopt(s, IPPROTO_IP, IP_DONTFRAGMENT, reinterpret_cast<char *>(&value), sizeof(value));
		}

		if(addr.is_v6()) {
			status = setsockopt(s, IPPROTO_IPV6, IPV6_DONTFRAG, reinterpret_cast<char *>(&value), sizeof(value));
		}

		return status == 0;
	}

	std::vector<Interface> GetCompatibleInterfaces(const IpAddress & forThisAddress) {
		if(forThisAddress.is_multicast()) {
			return std::vector<Interface>{};
		}

		std::vector<Interface> netifaces;
		
		if(forThisAddress.is_v4()) {
			netifaces = GetNativeNetworkInterfaces(AF_INET);
		} else if(forThisAddress.is_v6()) {
			netifaces = GetNativeNetworkInterfaces(AF_INET6);
		}

		auto it = std::remove_if(std::begin(netifaces), std::end(netifaces), [&forThisAddress](const Interface & neti) -> bool {
			return neti.address.is_loopback() != forThisAddress.is_loopback();
		});

		if(it != std::end(netifaces)) {
			netifaces.erase(it, std::end(netifaces));
		}

		return netifaces;
	}

}
