#pragma once

#include <Netcode/HandleDecl.h>
#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include <thread>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/Sync/LockGuards.hpp>

#include <Netcode/System/SystemClock.h>
#include "BasicPacket.hpp"

namespace Netcode::Network {
	enum class Error : unsigned {
		SUCCESS,
		ALREADY_RUNNING,
		CONNECTION_ALREADY_EXISTS,
		ADDRESS_RESOLUTION_FAILED,
		SOCK_ERR,
		BAD_API_CALL,
		NO_INTERFACE,
		BAD_REQUEST,
		TIMEDOUT,
		SERVER_IS_FULL
	};

	class ErrorCategory : public boost::system::error_category {
	public:
		const char * name() const noexcept
		{
			return "Netcode::Network";
		}

		std::string message(int v) const
		{
			Error value = static_cast<Error>(v);
			if(value == Error::SUCCESS)
				return "Operation completed successfully";
			if(value == Error::ADDRESS_RESOLUTION_FAILED)
				return "Address resolution failed";
			if(value == Error::NO_INTERFACE)
				return "No network interface";
			if(value == Error::BAD_REQUEST)
				return "Bad request";
			if(value == Error::TIMEDOUT)
				return "Message ACK window timed out";
			return "Netcode::Network error";
		}
	};

	inline boost::system::error_code make_error_code(Error e) {
		static ErrorCategory cat;
		return boost::system::error_code{ static_cast<int>(e), cat };
	}
}

namespace boost::system {
	template<>
	struct is_error_code_enum<Netcode::Network::Error> {
		static const bool value = true;
	};
}

namespace Netcode::Network {

	using IpAddress = boost::asio::ip::address;
	using UdpSocket = boost::asio::ip::udp::socket;
	using UdpResolver = boost::asio::ip::udp::resolver;
	using UdpEndpoint = boost::asio::ip::udp::endpoint;
	namespace Errc = boost::system::errc;
	using ErrorCode = boost::system::error_code;

	ErrorCode Bind(const boost::asio::ip::address & selfAddr, UdpSocket & udpSocket, uint32_t & port);

	/*
	 * Represents a basic Network interface
	 */
	struct Interface {
		boost::asio::ip::address address;
		uint64_t uplinkSpeedBps;
		uint64_t downlinkSpeedBps;
		uint32_t mtu;
		uint32_t netcodeRank; // how likely it is that we want to choose this interface
		std::wstring friendlyName;
		std::wstring description;
	};

	bool SetDontFragmentBit(UdpSocket & socket);
	
	std::vector<Interface> GetCompatibleInterfaces(const IpAddress & forThisAddress);

	class NetworkContext {
		boost::asio::io_context ioc;
		std::unique_ptr<boost::asio::io_context::work> work;
		std::vector<std::thread> workers;
	public:
		~NetworkContext();
		NetworkContext();

		void Start(uint8_t numThreads = 1);
		void Stop();

		boost::asio::io_context & GetImpl();
	};

	template<typename T>
	class MessageQueue {
		std::vector<T> recv;
		std::vector<T> send;
		SlimReadWriteLock srwLock;
	public:
		void GetOutgoingPackets(std::vector<T> & swapInto) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			swapInto.reserve(swapInto.size() + send.size());

			std::move(std::begin(send), std::end(send), std::back_inserter(swapInto));
			send.clear();
		}

		void GetIncomingPackets(std::vector<T> & swapInto) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			swapInto.reserve(swapInto.size() + recv.size());
			
			std::move(std::begin(recv), std::end(recv), std::back_inserter(swapInto));
			recv.clear();
		}

		void Send(T packet) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			send.emplace_back(std::move(packet));
		}

		void Received(T packet) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			recv.emplace_back(std::move(packet));
		}
	};

	// id, name, hash, is_banned
	using PlayerDbDataRow = std::tuple<int, std::string, std::string, bool>;

}

