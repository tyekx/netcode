#pragma once

#include <Netcode/HandleDecl.h>
#include <memory>
#include <mutex>
#include <vector>
#include <map>
#include <thread>
#include <future>
#include <boost/bind.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/Sync/LockGuards.hpp>

#include <Netcode/System/SystemClock.h>

namespace Netcode::Network {

	using udp_socket_t = boost::asio::ip::udp::socket;
	using udp_resolver_t = boost::asio::ip::udp::resolver;
	using udp_endpoint_t = boost::asio::ip::udp::endpoint;
	namespace Errc = boost::system::errc;
	using ErrorCode = boost::system::error_code;
	
	constexpr static uint32_t PACKET_STORAGE_SIZE = 65536;

	ErrorCode Bind(const boost::asio::ip::address & selfAddr, udp_socket_t & udpSocket, uint32_t & port);

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

	template<typename ProtocolType>
	class BasicPacket final {
	public:
		using EndpointType = typename ProtocolType::endpoint;

	private:
		EndpointType endpoint;
		Ref<uint8_t[]> data;
		size_t dataSize;
		Timestamp timestamp;
	public:
		BasicPacket(Ref<uint8_t[]> data, size_t size, EndpointType ep) : endpoint{ std::move(ep) }, data{ std::move(data) }, dataSize{ size }, timestamp{} { }
		BasicPacket(BasicPacket<ProtocolType> &&) noexcept = default;
		BasicPacket & operator=(BasicPacket<ProtocolType> &&) noexcept = default;
		BasicPacket(const BasicPacket<ProtocolType> &) = delete;
		BasicPacket & operator=(const BasicPacket<ProtocolType> &) = delete;

		[[nodiscard]]
		Ref<uint8_t[]> GetData() const {
			return data;
		}
		
		[[nodiscard]]
		uint8_t* GetDataPointer() const {
			return data.get();
		}

		[[nodiscard]]
		size_t GetDataSize() const {
			return dataSize;
		}

		void SetTimestamp(const Timestamp & ts) {
			timestamp = ts;
		}

		[[nodiscard]]
		Timestamp GetTimestamp() const {
			return timestamp;
		}
		
		void SetDataSize(size_t s) {
			dataSize = s;
		}

		[[nodiscard]]
		boost::asio::mutable_buffer GetMutableBuffer() const {
			return boost::asio::mutable_buffer{ GetDataPointer(), GetDataSize() };
		}

		[[nodiscard]]
		boost::asio::const_buffer GetConstBuffer() const {
			return boost::asio::const_buffer{ GetDataPointer(), GetDataSize() };
		}

		[[nodiscard]]
		EndpointType GetEndpoint() const {
			return endpoint;
		}
	};

	using TcpPacket = BasicPacket<boost::asio::ip::tcp>;
	using UdpPacket = BasicPacket<boost::asio::ip::udp>;


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

	class PacketStorage : public std::enable_shared_from_this<PacketStorage> {
	public:
	private:
		SlimReadWriteLock srwLock;
		std::vector<std::unique_ptr<uint8_t[]>> availableBuffers;

		std::unique_ptr<uint8_t[]> AllocateStorage();

		auto GetDestructor() {
			return [this, lifetime = shared_from_this()](uint8_t * bufferPointer) -> void {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
				std::unique_ptr<uint8_t[]> obj{ bufferPointer };
				
				if(availableBuffers.size() < 32) {
					availableBuffers.emplace_back(std::move(obj));
				}
			};
		}
		
	public:

		PacketStorage(uint32_t preallocatedBuffers = 32);

		Ref<uint8_t[]> GetBuffer();
	};

	// id, name, hash, is_banned
	using PlayerDbDataRow = std::tuple<int, std::string, std::string, bool>;

}

