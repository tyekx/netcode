#pragma once

#include <NetcodeFoundation/ErrorCode.h>
#include <Netcode/HandleDecl.h>
#include <memory>
#include <vector>
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
	class Node : public T {
	public:
		Node<T> * next;
		
		Node() : next { nullptr } { }

		using T::T;
		using T::operator=;
	};

	/**
	 * Invasive message queue to avoid list node allocation
	 * @tparam T will be wrapped into Node<T>
	 */
	template<typename T>
	class MessageQueue {
		SlimReadWriteLock srwLock;
		//std::atomic<Node<T>*> head;
		Node<T> * head;

	public:

		MessageQueue() : srwLock{}, head { nullptr } { }

		Node<T>* ConsumeAll() {
			ScopedExclusiveLock<SlimReadWriteLock> lock{ srwLock };
			Node<T> * p = head;
			head = nullptr;
			return p;
		}

		void Produce(Node<T>* msg) {
			ScopedExclusiveLock<SlimReadWriteLock> lock{ srwLock };
			msg->next = head;
			head = msg;
		}
		
		/*
		Node<T> * ConsumeAll() {
			for(;;) {
				Node<T> * currentHead = head.load(std::memory_order_acquire);

				if(head.compare_exchange_weak(currentHead, nullptr, std::memory_order_release, std::memory_order_relaxed)) {
					return currentHead;
				}
			}
		}

		void Produce(Node<T> * msg) {
			for(;;) {
				Node<T> * currentHead = head.load(std::memory_order_acquire);

				msg->next = currentHead;

				if(head.compare_exchange_weak(currentHead, msg, std::memory_order_release, std::memory_order_relaxed)) {
					return;
				}
			}
		}*/
	};

	// id, name, hash, is_banned
	using PlayerDbDataRow = std::tuple<int, std::string, std::string, bool>;

}

