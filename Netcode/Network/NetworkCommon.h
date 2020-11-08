#pragma once

#include <NetcodeFoundation/ErrorCode.h>
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
#include "NetworkDecl.h"

namespace Netcode::Network {

	namespace Errc = boost::system::errc;

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

		uint8_t GetActiveThreadCount() const;
		
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

