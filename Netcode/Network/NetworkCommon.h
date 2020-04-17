#pragma once

#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include <thread>
#include <future>
#include <boost/bind.hpp>
#include <boost/asio/ip/udp.hpp>

namespace Netcode::Network {

	using udp_socket_t = boost::asio::ip::udp::socket;
	using udp_resolver_t = boost::asio::ip::udp::resolver;
	using udp_endpoint_t = boost::asio::ip::udp::endpoint;
	namespace Errc = boost::system::errc;
	using ErrorCode = boost::system::error_code;

	constexpr static uint32_t PACKET_STORAGE_SIZE = 65536;

	class UdpStream : public std::enable_shared_from_this<UdpStream> {
	protected:
		udp_socket_t socket;
	public:
		udp_endpoint_t _interalReceivedFrom;
		UdpStream(udp_socket_t socket) : socket{ std::move(socket) }, _interalReceivedFrom{} { }

		template<typename CompletionHandler>
		void AsyncRead(boost::asio::mutable_buffer buffer, CompletionHandler handler) {
			socket.async_receive_from(buffer, _interalReceivedFrom, [pThis = shared_from_this(), handler](ErrorCode ec, size_t size) -> void{
				handler(ec, size, pThis->_interalReceivedFrom);
			});
		}

		template<typename CompletionHandler>
		void AsyncWrite(boost::asio::const_buffer buffer, udp_endpoint_t endpoint, CompletionHandler handler) {
			socket.async_send_to(buffer, endpoint, [pThis = shared_from_this(), handler](ErrorCode ec, size_t size) -> void {
				handler(ec, size);
			});
		}
	};

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

	struct UdpPacket {
		std::shared_ptr<uint8_t[]> data;
		boost::asio::ip::udp::endpoint endpoint;
		boost::asio::mutable_buffer mBuffer;

		UdpPacket() = default;

		UdpPacket(std::shared_ptr<uint8_t[]> storage) : data{ std::move(storage) }, endpoint{}, mBuffer{ boost::asio::buffer(data.get(), PACKET_STORAGE_SIZE) } { }
	};

	template<typename T>
	class MessageQueue {
		std::vector<T> recv;
		std::vector<T> send;
		std::mutex mutex;
	public:
		void GetOutgoingPackets(std::vector<T> & swapInto) {
			std::scoped_lock<std::mutex> lock{ mutex };

			swapInto.reserve(swapInto.size() + send.size());

			std::move(std::begin(send), std::end(send), std::back_inserter(swapInto));
			send.clear();
		}

		void GetIncomingPackets(std::vector<T> & swapInto) {
			std::scoped_lock<std::mutex> lock{ mutex };

			swapInto.reserve(swapInto.size() + recv.size());
			
			std::move(std::begin(recv), std::end(recv), std::back_inserter(swapInto));
			recv.clear();
		}

		void Send(T packet) {
			std::scoped_lock<std::mutex> lock{ mutex };
			send.push_back(packet);
		}

		void Received(T packet) {
			std::scoped_lock<std::mutex> lock{ mutex };

			recv.push_back(packet);
		}
	};

	class PacketStorage {
	public:
		using StorageType = std::shared_ptr<uint8_t[]>;
	private:
		std::vector<StorageType> availableBuffers;

		StorageType AllocateStorage();
	public:

		PacketStorage(uint32_t preallocatedBuffers = 32);

		void ReturnBuffer(StorageType storage);

		void ReturnBuffer(std::vector<StorageType> buffers);

		StorageType GetBuffer();
	};

	class ControlPacketStorage {
		struct Item {
			std::chrono::steady_clock::time_point first_sent_at;
			std::chrono::steady_clock::time_point last_sent_at;
			int32_t packetId;
			UdpPacket packet;
			std::function<void(ErrorCode)> completionHandler;

			Item(int32_t pid, UdpPacket pck, std::function<void(ErrorCode)> completionHandler) :
				first_sent_at{}, last_sent_at{}, packetId{ pid }, packet{ std::move(pck) }, completionHandler{ std::move(completionHandler) } { }
		};

		std::map<udp_endpoint_t, std::list<Item>> storage;
		std::recursive_mutex mutex;
	public:
		void CheckTimeouts(uint64_t durationMs);
		/*
		An expired item is an item that was not sent before, or its resend-timeout is expired,
		assumes that the call of this function means a send operation thus renewing its timestamps
		*/
		template<typename Func>
		void ForeachExpired(Func func, uint64_t resendTimeoutMs) {
			auto now = std::chrono::steady_clock::now();
			for(auto & i : storage) {
				if(i.second.empty()) {
					continue;
				}

				Item & item = i.second.front();

				auto timestamp = item.first_sent_at;

				if(timestamp.time_since_epoch() == std::chrono::steady_clock::duration::zero()) {
					// ok
					func(item.packet);
					item.first_sent_at = now;
					item.last_sent_at = now;
					continue;
				}

				timestamp = item.last_sent_at;

				if(std::chrono::duration_cast<std::chrono::milliseconds>(now - timestamp).count() > resendTimeoutMs) {
					func(item.packet);
					item.last_sent_at = now;
				}
			}
		}

		ErrorCode Acknowledge(const udp_endpoint_t & source, int32_t ackId);

		void Push(int32_t id, UdpPacket packet, std::function<void(ErrorCode)> completionHandler);
	};

	struct UserRow {
		int userId;
		std::string name;
		std::string hash;
		bool isBanned;
	};

}

