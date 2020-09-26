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

namespace Netcode::Network {

	using IpAddress = boost::asio::ip::address;
	using UdpSocket = boost::asio::ip::udp::socket;
	using UdpResolver = boost::asio::ip::udp::resolver;
	using UdpEndpoint = boost::asio::ip::udp::endpoint;
	namespace Errc = boost::system::errc;
	using ErrorCode = boost::system::error_code;
	
	constexpr static uint32_t PACKET_STORAGE_SIZE = 65536;

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

	template<typename ProtocolType, uint32_t NumBytes>
	class BasicPacket final {
	public:
		using EndpointType = typename ProtocolType::endpoint;

	private:
		EndpointType endpoint;
		size_t dataSize;
		Timestamp timestamp;
		uint8_t data[NumBytes];
	public:
		constexpr static uint32_t MAX_DATA_SIZE = NumBytes;
		
		BasicPacket() : endpoint{}, dataSize{ NumBytes }, timestamp{} { }
		BasicPacket(size_t size, EndpointType ep) : endpoint{ std::move(ep) },  dataSize{ size }, timestamp{} { }
		BasicPacket(BasicPacket<ProtocolType, NumBytes> &&) noexcept = default;
		BasicPacket & operator=(BasicPacket<ProtocolType, NumBytes> &&) noexcept = default;
		BasicPacket(const BasicPacket<ProtocolType, NumBytes> &) = delete;
		BasicPacket & operator=(const BasicPacket<ProtocolType, NumBytes> &) = delete;

		[[nodiscard]]
		uint8_t* GetData() {
			return data;
		}
		
		[[nodiscard]]
		const uint8_t * GetData() const {
			return data;
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
		boost::asio::mutable_buffer GetMutableBuffer() {
			return boost::asio::mutable_buffer{ static_cast<void*>(data), GetDataSize() };
		}

		[[nodiscard]]
		boost::asio::const_buffer GetConstBuffer() const {
			return boost::asio::const_buffer{ static_cast<const void *>(data), GetDataSize() };
		}

		void SetEndpoint(const EndpointType& ep) {
			endpoint = ep;
		}
		
		[[nodiscard]]
		const EndpointType & GetEndpoint() const {
			return endpoint;
		}

		[[nodiscard]]
		EndpointType & GetEndpoint() {
			return endpoint;
		}
	};

	using UdpPacket = BasicPacket<boost::asio::ip::udp, 1536>;


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

	template<typename T, typename ... StorageStateTypes>
	class PacketStorage : public std::enable_shared_from_this<PacketStorage<T, StorageStateTypes...>> {
		using Base = std::enable_shared_from_this<PacketStorage<T, StorageStateTypes...>>;
		using StateContainer = std::tuple<StorageStateTypes...>;
		
		SlimReadWriteLock srwLock;
		std::vector<std::unique_ptr<T>> availableBuffers;
		StateContainer storageState;

		auto GetDestructor() {
			return [lifetime = Base::shared_from_this(), this](T * bufferPointer) -> void {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
				std::unique_ptr<T> obj{ bufferPointer };
				
				if(availableBuffers.size() < 32) {
					availableBuffers.emplace_back(std::move(obj));
				}
			};
		}

		std::unique_ptr<T> Make() {
			return std::apply([](auto && ... args) -> std::unique_ptr<T> {
				return std::make_unique<T>(std::forward<decltype(args)>(args)...);
			}, storageState);
		}
		
	public:

		PacketStorage(uint32_t preallocatedBuffers, StorageStateTypes && ... args) : srwLock{}, availableBuffers{}, storageState{ std::forward_as_tuple(args...) }{
			availableBuffers.reserve(preallocatedBuffers * 2);

			for(uint32_t i = 0; i < preallocatedBuffers; ++i) {
				auto ptr = Make();
				availableBuffers.emplace_back(std::move(ptr));
			}
		}

		Ref<T> GetBuffer() {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			if(availableBuffers.empty()) {
				std::unique_ptr<T> b = Make();
				return Ref<T>{ b.release(), GetDestructor() };
			}

			auto ptr = std::move(availableBuffers.back());
			availableBuffers.pop_back();
			return Ref<T>{ ptr.release(), GetDestructor() };
		}
	};

	// id, name, hash, is_banned
	using PlayerDbDataRow = std::tuple<int, std::string, std::string, bool>;

}

