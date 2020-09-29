#pragma once

#include "NetworkCommon.h"
#include <Netcode/System/SystemClock.h>
#include <NetcodeProtocol/header.pb.h>
#include <NetcodeProtocol/netcode.pb.h>
#include <NetcodeFoundation/Exceptions.h>
#include <Netcode/Utility.h>
#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/Config.h>
#include <random>
#include "NetcodeFoundation/Enum.hpp"
#include <Netcode/System/FpsCounter.h>

#include <ppltasks.h>
#include <agents.h>
#include <ppl.h>

namespace Netcode::Network {

	enum class ConnectionState : uint32_t {
		INACTIVE = 0,
		DISCONNECTED = 1,
		REJECTED = 403,
		TIMEOUT = 404,
		INTERNAL_ERROR = 500,
		
		RESOLVING = 0x4000,
		CONNECTING = 0x8000,
		AUTHENTICATING = 0x8001,
		SYNCHRONIZING = 0x8002,
		ESTABLISHED = 0x10000
	};

	NETCODE_ENUM_CLASS_OPERATORS(ConnectionState)
	
	using GamePacket = BasicPacket<boost::asio::ip::udp, 65536>;
	using WaitableTimer = boost::asio::basic_waitable_timer<Netcode::ClockType>;



	struct FragmentData {
		uint8_t fragmentIndex;
		uint8_t fragmentCount;
		uint16_t sizeInBytes;

		FragmentData() : fragmentIndex{ 0 }, fragmentCount{ 0 }, sizeInBytes{ 0 } { }

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

	template<typename T, unsigned PAGE_SIZE = 32>
	class ManagedPool : public std::enable_shared_from_this<ManagedPool<T, PAGE_SIZE>> {

		struct Page {
			uint8_t data[PAGE_SIZE * sizeof(T)];
			uint32_t idx;

			Page() : idx{ 0 } {}
		};

		struct FreeListItem {
			FreeListItem * next;
		};

		FreeListItem * head;
		std::list< Page > pages;
		SlimReadWriteLock srwLock;

		using Base = std::enable_shared_from_this<ManagedPool<T, PAGE_SIZE>>;

	public:

		ManagedPool() : head{ nullptr }, pages{}, srwLock{} {
			pages.emplace_back();
		}

		NETCODE_CONSTRUCTORS_DELETE_COPY(ManagedPool);
		NETCODE_CONSTRUCTORS_DELETE_MOVE(ManagedPool);

		void FreeItem(T * ptr) {
			ptr->~T(); // this could actually take a while

			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			auto * p = reinterpret_cast<FreeListItem *>(ptr);
			p->next = head;
			head = p;
		}

		struct ManagedPoolDeleter {
			Ref<ManagedPool> self;

			void operator()(T * freedPtr) {
				self->FreeItem(freedPtr);
			}
		};

		using ObjectType = std::unique_ptr<T, ManagedPoolDeleter>;

		template<typename ... U>
		ObjectType Get(U && ... args) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			void * dst;

			if(head != nullptr) {
				dst = head;
				head = head->next;
			} else {
				auto & page = pages.back();

				if(page.idx == PAGE_SIZE) {
					pages.emplace_back();
					page = pages.back();
				}

				dst = page.data + sizeof(T) * page.idx;
				page.idx += 1;
			}

			return std::unique_ptr<T, ManagedPoolDeleter>{
				new (dst) T{ std::forward<U>(args)... },
					ManagedPoolDeleter{ Base::shared_from_this() }
			};
		}
	};

	struct MtuValue {
		constexpr static uint32_t MSB = (0x80000000);
		constexpr static uint32_t UDP_HEADER_SIZE = 8;
		constexpr static uint32_t IPV4_HEADER_SIZE = 20;
		constexpr static uint32_t IPV6_HEADER_SIZE = 40;

		uint32_t mtu;
		uint32_t maxPayloadSize;
	public:
		MtuValue(const MtuValue & rhs) = default;
		MtuValue & operator=(const MtuValue & rhs) = default;

		MtuValue(uint32_t v, bool isIpv4) {
			mtu = v;

			if((isIpv4 && v < 68) || (!isIpv4 && v < 576)) {
				throw OutOfRangeException{ "given MTU size is below the RFC specified values" };
			}

			if(isIpv4) {
				maxPayloadSize = mtu - IPV4_HEADER_SIZE - UDP_HEADER_SIZE;
			} else {
				maxPayloadSize = mtu - IPV6_HEADER_SIZE - UDP_HEADER_SIZE;
			}
		}

		uint32_t GetMaxPayloadSizeForUdp() const {
			return maxPayloadSize;
		}
	};


	enum class TransmissionState : uint32_t {
		UNKNOWN,
		TIMEOUT,
		ERROR_WHILE_SENDING,
		CANCELLED,
		SUCCESS
	};


	template<typename SockType, typename SockReaderWriter>
	class BasicSocket : public SockReaderWriter {
	protected:
		SockType socket;
	public:
		using SocketType = SockType;

		BasicSocket(SocketType s) : SockReaderWriter{ s }, socket { std::move(s) } {}
		
		const SockType & GetSocket() const {
			return socket;
		}

		SockType & GetSocket() {
			return socket;
		}

		template<typename ... ARGS>
		auto Send(ARGS && ... args) {
			return SockReaderWriter::Write(socket, std::forward<ARGS>(args)...);
		}
		
		template<typename ... ARGS>
		auto Receive(ARGS && ... args) {
			return SockReaderWriter::Read(socket, std::forward<ARGS>(args)...);
		}
	};

	template<typename SockType>
	class AsioSocketReadWriter {
	public:
		AsioSocketReadWriter(const SockType&) { }
		
		template<typename MutableBufferSequence, typename Endpoint, typename Handler>
		static void Read(SockType& socket, const MutableBufferSequence& buffers, Endpoint& remoteEndpoint, Handler&& handler) {
			socket.async_receive_from(buffers, remoteEndpoint, std::forward<Handler>(handler));
		}
		
		template<typename MutableBufferSequence, typename Handler>
		static void Read(SockType& socket, const MutableBufferSequence& buffers, Handler&& handler) {
			socket.async_receive(buffers, std::forward<Handler>(handler));
		}

		template<typename ConstBufferSequence, typename Handler>
		static void Write(SockType& socket, const ConstBufferSequence& buffers, Handler&& handler) {
			socket.async_send(buffers, handler);
		}
		
		template<typename ConstBufferSequence, typename Endpoint, typename Handler>
		static void Write(SockType & socket, const ConstBufferSequence & buffers, const Endpoint& remoteEndpoint, Handler && handler) {
			socket.async_send(buffers, remoteEndpoint, handler);
		}
	};

	template<typename SockType>
	class SyncAsioSocketHandler {
	public:
		SyncAsioSocketHandler(const SockType &) { }

		template<typename MutableBufferSequence, typename Endpoint>
		static size_t Read(SockType & socket, const MutableBufferSequence & buffers, Endpoint & remoteEndpoint, ErrorCode & ec) {
			return socket.receive_from(buffers, remoteEndpoint, 0, ec);
		}

		template<typename ConstBufferSequence, typename Endpoint>
		static size_t Write(SockType & socket, const ConstBufferSequence & buffers, const Endpoint & remoteEndpoint, ErrorCode  &ec) {
			return socket.send_to(buffers, remoteEndpoint, 0, ec);
		}
		
		template<typename MutableBufferSequence, typename Endpoint, typename Handler>
		static void Read(SockType & socket, const MutableBufferSequence & buffers, Endpoint & remoteEndpoint, Handler && handler) {
			ErrorCode ec;
			size_t s = socket.receive_from(buffers, remoteEndpoint, 0, ec);
			handler(ec, s);
		}

		template<typename MutableBufferSequence, typename Handler>
		static void Read(SockType & socket, const MutableBufferSequence & buffers, Handler && handler) {
			ErrorCode ec;
			size_t s = socket.receive(buffers, 0, ec);
			handler(ec, s);
		}

		template<typename ConstBufferSequence, typename Handler>
		static void Write(SockType & socket, const ConstBufferSequence & buffers, Handler && handler) {
			ErrorCode ec;
			size_t s = socket.send(buffers, 0, ec);
			handler(ec, s);
		}

		template<typename ConstBufferSequence, typename Endpoint, typename Handler>
		static void Write(SockType & socket, const ConstBufferSequence & buffers, const Endpoint & remoteEndpoint, Handler && handler) {
			ErrorCode ec;
			size_t s = socket.send_to(buffers, remoteEndpoint, 0, ec);
			handler(ec, s);
		}
	};

	template<typename SockType>
	class SharedAsioSocketReadWriter {
		SlimReadWriteLock srwLock;
	public:
		SharedAsioSocketReadWriter(const SockType &) { }

		NETCODE_CONSTRUCTORS_DELETE_MOVE(SharedAsioSocketReadWriter);
		NETCODE_CONSTRUCTORS_DELETE_COPY(SharedAsioSocketReadWriter);
		
		template<typename MutableBufferSequence, typename Endpoint, typename Handler>
		void Read(SockType & socket, const MutableBufferSequence & buffers, Endpoint & remoteEndpoint, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			socket.async_receive_from(buffers, remoteEndpoint, std::forward<Handler>(handler));
		}

		template<typename MutableBufferSequence, typename Handler>
		void Read(SockType & socket, const MutableBufferSequence & buffers, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			socket.async_receive(buffers, std::forward<Handler>(handler));
		}

		template<typename ConstBufferSequence, typename Handler>
		void Write(SockType & socket, const ConstBufferSequence & buffers, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			socket.async_send(buffers, handler);
		}

		template<typename ConstBufferSequence, typename Endpoint, typename Handler>
		void Write(SockType & socket, const ConstBufferSequence & buffers, const Endpoint & remoteEndpoint, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			socket.async_send(buffers, remoteEndpoint, handler);
		}
	};
	
	template<typename SockType>
	class DebugSharedAsioSocketReadWriter {
		SlimReadWriteLock srwLock;
		boost::asio::executor executor;
		Ref<ManagedPool<WaitableTimer>> fakeLagTimers;
		Duration fakeLagAvgDuration;
		std::default_random_engine randomEngine;
		std::normal_distribution<double> fakeLagDistribution;

		Duration SampleFakeLag() {
			if(fakeLagAvgDuration > Duration{}) {
				double d = fakeLagDistribution(randomEngine);

				return std::chrono::duration_cast<Duration>(std::chrono::duration<double, std::milli>(d));
			}
			return Duration{};
		}
	public:
		DebugSharedAsioSocketReadWriter(SockType & sock) : srwLock{}, executor{ sock.get_executor() },
			fakeLagTimers{ std::make_shared<ManagedPool<WaitableTimer>>() }, fakeLagAvgDuration{}, randomEngine{},
			fakeLagDistribution{} {

			uint32_t fakeLagAvgDur = 0;
			uint32_t fakeLagSigma = 0;
			try {
				fakeLagAvgDur = Config::Get<uint32_t>(L"network.fakeLagAvg:u32");
				fakeLagSigma = Config::Get<uint32_t>(L"network.fakeLagSigma:u32");
			} catch(OutOfRangeException & e) { }

			fakeLagAvgDuration = std::chrono::duration<uint64_t, std::milli>(fakeLagAvgDur);

			double sigma = std::max(static_cast<double>(fakeLagSigma), 0.001);

			fakeLagDistribution = std::normal_distribution<double>(static_cast<double>(fakeLagAvgDur), sigma);
		}

		NETCODE_CONSTRUCTORS_DELETE_MOVE(DebugSharedAsioSocketReadWriter);
		NETCODE_CONSTRUCTORS_DELETE_COPY(DebugSharedAsioSocketReadWriter);
		
		template<typename MutableBufferSequence, typename Endpoint, typename Handler>
		void Read(SockType & socket, const MutableBufferSequence & buffers, Endpoint & remoteEndpoint, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			/*
			 * Its enough to have the send delay in a symmetrical manner,
			 * shows up better in wireshark aswell
			 */
			socket.async_receive_from(buffers, remoteEndpoint, std::forward<Handler>(handler));
		}

		template<typename MutableBufferSequence, typename Handler>
		void Read(SockType & socket, const MutableBufferSequence & buffers, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			/*
			 * Its enough to have the send delay in a symmetrical manner,
			 * shows up better in wireshark aswell
			 */
			socket.async_receive(buffers, std::forward<Handler>(handler));
		}

		template<typename ConstBufferSequence, typename Handler>
		void Write(SockType & socket, const ConstBufferSequence & buffers, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			auto fakeLagTimer = fakeLagTimers->Get(executor);
			fakeLagTimer->expires_from_now(SampleFakeLag());
			fakeLagTimer->async_wait([this, &socket, buffers, l = std::move(fakeLagTimer), h = std::move(handler)]
			(const ErrorCode & ec) mutable -> void {
				ScopedExclusiveLock<SlimReadWriteLock> callbackScopedLock{ srwLock };
				socket.async_send(buffers, std::move(h));
			});
		}

		template<typename ConstBufferSequence, typename Endpoint, typename Handler>
		void Write(SockType & socket, const ConstBufferSequence & buffers, const Endpoint & remoteEndpoint, Handler && handler) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			auto fakeLagTimer = fakeLagTimers->Get(executor);
			fakeLagTimer->expires_from_now(SampleFakeLag());
			fakeLagTimer->async_wait([this, &socket, buffers, &remoteEndpoint, l = std::move(fakeLagTimer), h = std::move(handler)]
			(const ErrorCode & ec) mutable -> void {
				ScopedExclusiveLock<SlimReadWriteLock> callbackScopedLock{ srwLock };
				socket.async_send_to(buffers, remoteEndpoint, std::move(h));
			});
		}
	};

	template<typename SockType>
	using BasicAsioSocket = BasicSocket<SockType, AsioSocketReadWriter<SockType>>;
	
	template<typename SockType>
	using BasicSharedAsioSocket = BasicSocket<SockType, SharedAsioSocketReadWriter<SockType>>;

	using SharedUdpSocket = BasicSharedAsioSocket<boost::asio::ip::udp::socket>;

	class ConnectionBase : public std::enable_shared_from_this<ConnectionBase> {
	public:
		MessageQueue<Protocol::Header> sharedControlQueue;
		// shared between service and consumer
		MessageQueue<Protocol::Update> sharedQueue;
		// must be kept unchanged for a connection
		UdpEndpoint endpoint;
		// handled by the service
		uint32_t localSequence;
		// handled by the service
		uint32_t remoteSequence;
		// state of the connection, handled by the consumer
		Enum<ConnectionState> state;
	};

	class ConnectionStorage {
		SlimReadWriteLock srwLock;
		std::vector<Ref<ConnectionBase>> connections;
	public:
		void RemoveConnection(Ref<ConnectionBase> conn) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			auto it = std::remove(std::begin(connections), std::end(connections), conn);

			connections.erase(it, std::end(connections));
		}
		
		void AddConnection(Ref<ConnectionBase> conn) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			auto it = std::find_if(std::begin(connections), std::end(connections), [&conn](const Ref<ConnectionBase> & c) -> bool {
				return c == conn;
			});

			if(it == std::end(connections)) {
				connections.emplace_back(std::move(conn));
			}
		}
		
		Ref<ConnectionBase> GetConnectionByEndpoint(const UdpEndpoint& ep) {
			ScopedSharedLock<SlimReadWriteLock> scopedLock{ srwLock };

			auto it = std::find_if(std::begin(connections), std::end(connections), [&ep](const Ref<ConnectionBase> & c)-> bool {
				return c->endpoint == ep;
			});

			if(it != std::end(connections)) {
				return *it;
			}

			return nullptr;
		}
	};

	class Request {
	public:
		UdpEndpoint sourceEndpoint;
		Protocol::Header header;

		NETCODE_CONSTRUCTORS_NO_COPY(Request);

		Request(const UdpEndpoint & ep, Protocol::Header h) : sourceEndpoint{ ep }, header{ std::move(h) } { }
	};

	struct ResendArgs {
		Duration resendInterval;
		uint32_t maxAttempts;

		constexpr ResendArgs() : resendInterval{}, maxAttempts{} {}

		constexpr ResendArgs(uint32_t resendIntervalMs, uint32_t maxAttempts) :
			resendInterval{ std::chrono::milliseconds(resendIntervalMs) }, maxAttempts{ maxAttempts } { }
	};


	class ProtocolConfig {
		constexpr uint32_t GetIndexOf(Protocol::MessageType messageType) {
			switch(messageType) {
				default: return 0;
				case Protocol::MessageType::HEARTBEAT: return 1;
				case Protocol::MessageType::NAT_DISCOVERY_RESPONSE: return 2;
				case Protocol::MessageType::NAT_DISCOVERY_REQUEST: return 3;
				case Protocol::MessageType::CONNECT_PUNCHTHROUGH: return 4;
				case Protocol::MessageType::CONNECT_REQUEST: return 5;
				case Protocol::MessageType::CONNECT_RESPONSE: return 6;
				case Protocol::MessageType::PMTU_DISCOVERY: return 7;
				case Protocol::MessageType::DISCONNECT: return 8;
				case Protocol::MessageType::DISCONNECT_NOTIFY: return 9;
				case Protocol::MessageType::REG_HOST_REQUEST: return 10;
				case Protocol::MessageType::REG_HOST_RESPONSE: return 11;
			}
		}

		inline static ResendArgs args[] = {
			ResendArgs{ 0, 0 },
			ResendArgs{ 60000, 5 },
			ResendArgs{ 500, 5 },
			ResendArgs{ 500, 5 },
			ResendArgs{ 1000, 10 },
			ResendArgs{ 1000, 5 },
			ResendArgs{ 1000, 5 },
			ResendArgs{ 500, 3 },
			ResendArgs{ 1000, 5 },
			ResendArgs{ 1000, 5 },
			ResendArgs{ 500, 5 },
			ResendArgs{ 500, 5 },
		};
	public:

		ResendArgs GetArgsFor(Protocol::MessageType messageType) {
			return args[GetIndexOf(messageType)];
		}
	};

	struct SocketOperationResult {
		size_t numBytes;
		ErrorCode errorCode;
	};
	
	struct TrResult {
		TransmissionState state; // write once
		uint32_t sequence; // const
		uint32_t fragments; // const
		uint32_t attempts; // resend only
		uint32_t sentBytes; // resend only
		uint32_t dataSize; // const
		ErrorCode errorIfAny; // resend only

		TrResult() : state{ TransmissionState::UNKNOWN }, sequence{ 0 }, fragments{ 0 }, attempts{ 0 }, sentBytes{ 0 }, dataSize{ 0 }, errorIfAny{} { }
	};

	struct UdpAck {
		uint32_t sequence;
		UdpEndpoint endpoint;
	};

	struct HandleBase {
		concurrency::task_completion_event<TrResult> completionEvent;
		TrResult result;
		HandleBase() : completionEvent{} { }
	};
	
	struct RawUdpHandle : public HandleBase {
		Ref<UdpPacket> data;
	};

	struct ReliableUdpHandle : public HandleBase {
		uint32_t sequence;
		ResendArgs resendArgs;
		Ref<UdpPacket> data;
	};

	struct FragmentationContext {
		Ref<UdpPacket> headerContent;
		Ref<GamePacket> gameContent;
		std::array<boost::asio::const_buffer, 2> constBuffers;

		uint32_t GetHeaderSize() const {
			return static_cast<uint32_t>(constBuffers[0].size());
		}

		uint32_t GetDataSize() const {
			return static_cast<uint32_t>(constBuffers[1].size());
		}

		FragmentationContext(Ref<UdpPacket> hdrContent, uint32_t hdrOffset, uint32_t hdrSize,
			Ref<GamePacket> msgContent, uint32_t msgOffset, uint32_t msgSize) :
			headerContent{ std::move(hdrContent) },
			gameContent{ std::move(msgContent) },
			constBuffers{
				boost::asio::const_buffer{ headerContent->GetData() + hdrOffset, hdrSize },
				boost::asio::const_buffer{ gameContent->GetData() + msgOffset, msgSize }
		} { }
	};

	struct FragmentedUdpHandle : public HandleBase {
		using FragCtxHandle = ManagedPool<FragmentationContext>::ObjectType;

		std::array<FragCtxHandle, 16> fragments;
		uint32_t numFragments;

		FragmentedUdpHandle() : HandleBase{}, fragments {}, numFragments{} { }
	};

	struct SocketWriteHandle {
		concurrency::task_completion_event<SocketOperationResult> completionEvent;
		std::array<boost::asio::const_buffer, 2> buffers;
		UdpEndpoint endpoint;
		Ref<UdpPacket> lifetime;
		Ref<GamePacket> lifetime2;
	};

	class WorkToken {
		int32_t done;

	public:
		WorkToken() : done{ 0 } { }
		
		void SetDone() {
			done = 1;
		}

		bool IsDone() const {
			return done != 0;
		}
	};

	class AgentBase : public concurrency::agent {
	protected:
		Ptr<WorkToken> token;
		
		virtual void run() override {
			while(!token->IsDone()) {
				RunImpl();
			}
		}
	protected:
		virtual void RunImpl() = 0;

	public:
		AgentBase(Ptr<WorkToken> wt) : concurrency::agent{}, token{ wt } {}
	};

	template<typename SockType>
	class UdpSocketWriterAgent : public AgentBase {
		concurrency::ISource<SocketWriteHandle> & source;
		
		Ptr<SockType> socket;
	public:

		UdpSocketWriterAgent(concurrency::ISource<SocketWriteHandle> & source, Ptr<SockType> sock, Ptr<WorkToken> wt) :
			AgentBase{ wt }, source{ source }, socket{ sock } {
			
		}

		virtual void RunImpl() override {
			SocketOperationResult sor;
			auto input = concurrency::receive(source);

			/*std::ostringstream oss;
			oss << input.buffers[0].data() << ":" << input.buffers[0].size() << " sending to: " << input.endpoint.address().to_string() << ":" << std::to_string(input.endpoint.port());
			
			Log::Debug("{0}", oss.str());*/
			
			sor.numBytes = socket->Send(input.buffers, input.endpoint, sor.errorCode);

			if(sor.errorCode) {
				Log::Debug("Err while writing: {0}", sor.errorCode.message());
			}
			
			input.completionEvent.set(std::move(sor));
		}
	};

	struct AckHandle {
		concurrency::task_completion_event<uint32_t> completionEvent;
		UdpEndpoint endpoint;
		uint32_t sequence;

		AckHandle() = default;
		AckHandle(concurrency::task_completion_event<uint32_t> evt, const UdpEndpoint& ep, uint32_t seq) :
			completionEvent{ std::move(evt) }, endpoint{ ep }, sequence{ seq } { }
	};
	
	class UdpAckAgent : public AgentBase {
	private:
		concurrency::ISource<AckHandle> & sideSource;
		concurrency::ISource<UdpAck> & mainSource;
	public:

		UdpAckAgent(concurrency::ISource<AckHandle> & sideSource, concurrency::ISource<UdpAck> & mainSource, Ptr<WorkToken> token) :
			AgentBase{ token }, sideSource{ sideSource }, mainSource{ mainSource }  {
			
		}
		
	protected:
		virtual void RunImpl() override {
			UdpAck udpAck = concurrency::receive(mainSource);

			AckHandle handle;

			Log::Debug("ACK received");

			if(concurrency::try_receive(sideSource, handle, [&udpAck](const AckHandle & h) -> bool {
				return h.endpoint == udpAck.endpoint && h.sequence == udpAck.sequence;
			})) {
				bool b = handle.completionEvent.set(0);
				Log::Debug("Tried receive success, setting result: {0}", static_cast<int>(b));
			}
		}
	};
	
	class ReliableUdpAgent : public AgentBase {
	private:
		concurrency::ISource<ReliableUdpHandle> & source;
		concurrency::ITarget<SocketWriteHandle> & sockWriteTarget;
		concurrency::ITarget<AckHandle> & waitingForAckTarget;
		concurrency::ITarget<UdpAck> & fakeAckTarget;
		
	public:

		explicit ReliableUdpAgent(	concurrency::ISource<ReliableUdpHandle> & src,
									concurrency::ITarget<SocketWriteHandle> & swt,
									concurrency::ITarget<AckHandle> & wfat,
									concurrency::ITarget<UdpAck> & fat,
									Ptr<WorkToken> token) :
			AgentBase{ token }, source{ src }, sockWriteTarget{ swt }, waitingForAckTarget{ wfat }, fakeAckTarget{ fat } {

		}
	protected:

		void SendToSocket(concurrency::task_completion_event<uint32_t> evt, ReliableUdpHandle handle, uint32_t currentAttempt) {
			SocketWriteHandle swh;
			swh.endpoint = handle.data->GetEndpoint();
			swh.buffers[0] = handle.data->GetConstBuffer();
			concurrency::send(sockWriteTarget, swh);
			concurrency::create_task(swh.completionEvent).then([evt, currentAttempt, mainEvent = handle.completionEvent](SocketOperationResult sor) -> void {
				if(sor.errorCode) {
					TrResult tr;
					tr.state = TransmissionState::ERROR_WHILE_SENDING;
					tr.errorIfAny = sor.errorCode;
					tr.sentBytes = sor.numBytes;
					mainEvent.set(tr);
					evt.set(currentAttempt);
				}
			});
		}

		void SendToAck(concurrency::task_completion_event<uint32_t> evt, ReliableUdpHandle handle) {
			AckHandle a{ std::move(evt), handle.data->GetEndpoint(), handle.sequence };
			concurrency::send(waitingForAckTarget, a);
		}
		
		virtual void RunImpl() override {
			ReliableUdpHandle handle = concurrency::receive(source);

			int64_t ms = std::chrono::duration_cast<std::chrono::milliseconds>(handle.resendArgs.resendInterval).count();
			uint32_t maxAttempts = handle.resendArgs.maxAttempts;

			concurrency::task_completion_event<uint32_t> tce;
			
			auto callback = std::make_shared<concurrency::call<ReliableUdpHandle>>([this, i = 1u, maxAttempts, tce](ReliableUdpHandle msg) mutable -> void {
				uint32_t currentAttempts = i;

				if(++i >= maxAttempts) {
					tce.set(currentAttempts); // ready for cleanup
				} else {
					SendToSocket(tce, msg, i);
				}
			});

			SendToAck(tce, handle);
			SendToSocket(tce, handle, 1);

			auto timer = std::make_shared<concurrency::timer<ReliableUdpHandle>>(static_cast<uint32_t>(ms), handle, callback.get(), true);

			timer->start();

			concurrency::create_task(tce).then([this, callback, timer, handle](uint32_t result) -> void {
				timer->unlink_target(callback.get());
				timer->stop();

				Log::Debug("Timer stopping");
				
				TrResult tr;
				if(result == 0) {
					tr.state = TransmissionState::SUCCESS;
					Log::Debug("Set state to success");
				} else {
					tr.state = TransmissionState::TIMEOUT;
					Log::Debug("Set state to timeout");
					tr.attempts = result;
					UdpAck fakeAck;
					fakeAck.endpoint = handle.data->GetEndpoint();
					fakeAck.sequence = handle.sequence;
					concurrency::send(fakeAckTarget, fakeAck);
				}
				
				handle.completionEvent.set(tr);
			});
		}
	};

	struct GameMessageHandle : public HandleBase {
		uint32_t sequence;
		Ref<GamePacket> gamePacket; // serialized game data
		Ref<UdpPacket> headerPacket; // empty header buffer
	};
	
	class UdpFragmenterAgent : public AgentBase {
	private:
		concurrency::ISource<GameMessageHandle> & source;
		concurrency::ITarget<SocketWriteHandle> & sockWriteTarget;
		uint32_t mtu;

	public:
		UdpFragmenterAgent(concurrency::ISource<GameMessageHandle> & source, concurrency::ITarget<SocketWriteHandle> & sockWt, Ptr<WorkToken> token) : AgentBase{ token },
			source{ source }, sockWriteTarget{ sockWt }, mtu{ 1280 } {

		}

		void SetMtu(uint32_t maxTrUnit) {
			mtu = maxTrUnit;
		}
		
	protected:
		virtual void RunImpl() override {
			GameMessageHandle gmh = concurrency::receive(source);

			Protocol::Header protoHeader;
			protoHeader.set_sequence(gmh.sequence);
			protoHeader.set_type(Protocol::MessageType::GAME);

			const uint32_t serializedHeaderSize = protoHeader.ByteSizeLong();
			const uint32_t totalHeaderSize = serializedHeaderSize + 2 * sizeof(google::protobuf::uint32);
			const uint32_t totalDataSizeInBytes = static_cast<uint32_t>(gmh.gamePacket->GetDataSize());
			const uint32_t localMtu = mtu;
			const uint32_t maxPayloadSize = localMtu - totalHeaderSize;

			if(localMtu < totalHeaderSize) {
				throw UndefinedBehaviourException{ "MTU too small" };
			}
			
			const uint32_t numFragments = totalDataSizeInBytes / maxPayloadSize + (std::min(totalDataSizeInBytes % maxPayloadSize, 1u));

			if(numFragments > std::numeric_limits<uint8_t>::max()) {
				throw UndefinedBehaviourException{ "Too many fragments" };
			}

			std::vector<concurrency::task<SocketOperationResult>> tasks;
			tasks.reserve(numFragments);

			uint32_t remDataSize = totalDataSizeInBytes;
			uint32_t dataOffset = 0;
			uint32_t headerOffset = 0;

			google::protobuf::io::ArrayOutputStream outStream{ gmh.headerPacket->GetData(), static_cast<int32_t>(gmh.headerPacket->GetDataSize()) };
			google::protobuf::io::CodedOutputStream codedOutStream{ &outStream };
			
			for(uint32_t i = 0; i < numFragments; i++) {
				FragmentData fd;
				fd.sizeInBytes = std::min(remDataSize, maxPayloadSize);
				fd.fragmentIndex = static_cast<uint8_t>(i);
				fd.fragmentCount = static_cast<uint8_t>(numFragments);

				codedOutStream.WriteLittleEndian32(serializedHeaderSize);
				if(!protoHeader.SerializeToCodedStream(&codedOutStream)) {
					Log::Debug("Failed to serialize header");
				}
				codedOutStream.WriteLittleEndian32(fd.Pack());
				
				SocketWriteHandle swh;
				swh.buffers[0] = boost::asio::const_buffer{ gmh.headerPacket->GetData() + headerOffset, totalHeaderSize };
				swh.buffers[1] = boost::asio::const_buffer{ gmh.gamePacket->GetData() + dataOffset, fd.sizeInBytes };
				swh.endpoint = gmh.gamePacket->GetEndpoint();
				swh.lifetime = gmh.headerPacket;
				swh.lifetime2 = gmh.gamePacket;
				tasks.emplace_back(swh.completionEvent);
				concurrency::send(sockWriteTarget, std::move(swh));

				remDataSize -= fd.sizeInBytes;
				dataOffset += fd.sizeInBytes;
				headerOffset += totalHeaderSize;
			}

			concurrency::when_all(std::begin(tasks), std::end(tasks)).then([gmh](std::vector<SocketOperationResult> results) -> void {
				TrResult finalResult;
				finalResult.state = TransmissionState::SUCCESS;
				for(const SocketOperationResult & t : results) {
					finalResult.sentBytes += t.numBytes;
					
					if(t.errorCode) {
						if(!finalResult.errorIfAny) {
							finalResult.errorIfAny = t.errorCode;
							finalResult.state = TransmissionState::ERROR_WHILE_SENDING;
						}
					}
				}
				gmh.completionEvent.set(finalResult);
			});
		}
	};

	template<typename SockType>
	class UdpSocketReaderAgent : public AgentBase {
	public:
		Ptr<SockType> socket;
		Ref<PacketStorage<UdpPacket>> packetStorage;
		concurrency::ITarget<Ref<UdpPacket>> & readTarget;
		uint32_t consecutiveErrorCount;

		UdpSocketReaderAgent(Ptr<SockType> socket, Ref<PacketStorage<UdpPacket>> packetStorage, concurrency::ITarget<Ref<UdpPacket>> & readTarget, Ptr<WorkToken> wt) :
			AgentBase{ wt }, socket { socket }, packetStorage{ std::move(packetStorage) }, readTarget{ readTarget }, consecutiveErrorCount{ 0 } { }
		
	protected:
		virtual void RunImpl() override {
			Ref<UdpPacket> udpPacket = packetStorage->GetBuffer();
			udpPacket->SetDataSize(UdpPacket::MAX_DATA_SIZE);
			SocketOperationResult sor;
			sor.numBytes = socket->Receive(udpPacket->GetMutableBuffer(), udpPacket->GetEndpoint(), sor.errorCode);
			udpPacket->SetDataSize(sor.numBytes);

			if(sor.errorCode) {
				Log::Debug("Error while reading: {0}", sor.errorCode.message());
				consecutiveErrorCount++;

				if(consecutiveErrorCount > 5) {
					Log::Error("5 consequtive read errors occured, stopping");
					token->SetDone();
				}
			} else {
				consecutiveErrorCount = 0;
				//auto ep = udpPacket->GetEndpoint();
				//std::string addrPort = ep.address().to_string() + ":" + std::to_string(ep.port());
				//Log::Debug("Raw read: {0}", addrPort);
				
				concurrency::send(readTarget, std::move(udpPacket));
			}
		}
	};

	struct UdpFragment {
		uint32_t sequence;
		FragmentData fragmentData;
		uint32_t dataOffset;
		Ref<UdpPacket> data;
	};
	
	class ParserAgent : public AgentBase {
	public:
		Ref<PacketStorage<UdpPacket>> packetStorage;
		concurrency::ISource<Ref<UdpPacket>> & source;
		concurrency::ITarget<UdpAck> & ackTarget;
		concurrency::ITarget<SocketWriteHandle> & ackWriteTarget;
		concurrency::ITarget<UdpFragment> & fragWriteTarget;

		ParserAgent(Ref<PacketStorage<UdpPacket>> pktStorage,
			concurrency::ISource<Ref<UdpPacket>> & source,
			concurrency::ITarget<UdpAck> & ackTarget,
			concurrency::ITarget<SocketWriteHandle> & ackWriteTarget,
			concurrency::ITarget<UdpFragment> & fragWriteTarget,
			Ptr<WorkToken> wt) :
			AgentBase{ wt }, packetStorage{ std::move(pktStorage) }, source{ source }, ackTarget{ ackTarget }, ackWriteTarget{ ackWriteTarget }, fragWriteTarget{ fragWriteTarget } { }
		
	protected:
		void SendAck(uint32_t seq, const UdpEndpoint & ep) {
			Ref<UdpPacket> pkt = packetStorage->GetBuffer();
			pkt->SetDataSize(UdpPacket::MAX_DATA_SIZE);
			pkt->SetEndpoint(ep);
			Protocol::Header h;
			h.set_sequence(seq);
			h.set_type(Protocol::MessageType::ACKNOWLEDGE);

			google::protobuf::io::ArrayOutputStream aos{ pkt->GetData(), static_cast<int32_t>(pkt->GetDataSize()) };
			google::protobuf::io::CodedOutputStream codedOutStream{ &aos };

			uint32_t serializedHeaderSize = static_cast<uint32_t>(h.ByteSizeLong());
			
			codedOutStream.WriteLittleEndian32(serializedHeaderSize);
			if(!h.SerializeToCodedStream(&codedOutStream)) {
				Log::Debug("Failed to serialize ACK message");
				return;
			}
			pkt->SetDataSize(serializedHeaderSize + sizeof(google::protobuf::uint32));

			SocketWriteHandle swh;
			swh.endpoint = pkt->GetEndpoint();
			swh.buffers[0] = pkt->GetConstBuffer();
			swh.lifetime = pkt;

			concurrency::send(ackWriteTarget, std::move(swh));
		}
		
		bool ParseNetcodeHeader(google::protobuf::io::CodedInputStream * codedStream, uint32_t & headerSize, Protocol::Header& header, FragmentData & fd) {

			uint32_t serializedHeaderSize = std::numeric_limits<uint32_t>::max();
			if(!codedStream->ReadLittleEndian32(&serializedHeaderSize)) {
				Log::Debug("Failed to read serialized header size");
				return false;
			}

			auto limit = codedStream->PushLimit(static_cast<int32_t>(serializedHeaderSize));

			Protocol::Header h;
			if(!h.ParseFromCodedStream(codedStream)) {
				Log::Debug("Failed to parse header");
				return false;
			} else {
				if(h.ByteSizeLong() != serializedHeaderSize) {
					Log::Debug("Protocol misuse");
					return false;
				}
			}

			codedStream->PopLimit(limit);

			uint32_t fragData = 0;
			FragmentData tmpFd;

			if(h.type() == Protocol::MessageType::GAME) {
				if(!codedStream->ReadLittleEndian32(&fragData)) {
					Log::Debug("Failed to read fragmentation data");
					return false;
				}
				
				if(fragData == 0) {
					Log::Debug("Fragment data expected to be non-zero");
					return false;
				}
				
				tmpFd.Unpack(fragData);
				
				if(tmpFd.fragmentIndex >= tmpFd.fragmentCount) {
					Log::Debug("Invalid fragment index");
					return false;
				}
			}

			headerSize = serializedHeaderSize;
			header = std::move(h);
			fd = tmpFd;
			return true;
		}
		
		virtual void RunImpl() override {
			Ref<UdpPacket> handle = concurrency::receive(source);

			if(handle == nullptr) {
				return;
			}
			
			google::protobuf::io::ArrayInputStream ais{ handle->GetData(), static_cast<int32_t>(handle->GetDataSize()) };
			google::protobuf::io::CodedInputStream cis{ &ais };

			uint32_t headerSize;
			Protocol::Header header;
			FragmentData fragData;
			if(!ParseNetcodeHeader(&cis, headerSize, header, fragData)) {
				return;
			}
			
			if(header.type() == Protocol::MessageType::ACKNOWLEDGE) {

				UdpAck ack;
				ack.endpoint = handle->GetEndpoint();
				ack.sequence = header.sequence();
				concurrency::send(ackTarget, ack);
				return;
				
			}

			if(header.type() == Protocol::MessageType::GAME) {
				
				UdpFragment udpFrag;
				udpFrag.data = std::move(handle);
				udpFrag.fragmentData = fragData;
				udpFrag.sequence = header.sequence();
				udpFrag.dataOffset = static_cast<uint32_t>(cis.CurrentPosition());
				concurrency::send(fragWriteTarget, udpFrag);
				return;
				
			}

			if((header.type() & 0x1) == 0x1) {

				Log::Debug("Responsing with an ACK");
				SendAck(header.sequence(), handle->GetEndpoint());
				
			}

			// ---
		}
	};
	
	class DefragmenterAgent : public AgentBase {
		ConnectionStorage * connStorage;
		concurrency::ISource<UdpFragment> & fragmentSource;
		
		class DefragCtx {
			uint32_t sequence;
			UdpEndpoint endpoint;
			Timestamp createdAt;
			std::unique_ptr<UdpFragment[]> fragments;
			uint32_t receivedFragments;
			uint32_t numFragments;
			Ref<GamePacket> defragTarget;

		public:

			const Timestamp& GetTimestamp() const {
				return createdAt;
			}
			
			bool TryDefragment(Protocol::Update& update) {
				if(!IsComplete()) {
					return false;
				}

				uint8_t * const dstBase = defragTarget->GetData();
				const uint32_t dstSize = defragTarget->GetDataSize();
				uint32_t dstOffset = 0;
				for(uint32_t i = 0; i < numFragments; i++) {
					const UdpFragment * f = fragments.get() + i;
					const uint32_t numBytes = f->fragmentData.sizeInBytes;

					if((dstOffset + numBytes) > dstSize || numBytes == 0) {
						Log::Debug("Error while defragmenting");
						return false;
					}
					
					memcpy(dstBase + dstOffset, f->data->GetData() + f->dataOffset, numBytes);
					dstOffset += numBytes;
				}

				defragTarget->SetDataSize(dstOffset);

				if(!update.ParseFromArray(defragTarget->GetData(), static_cast<int32_t>(defragTarget->GetDataSize()))) {
					Log::Debug("Failed to parse defragmented game message");
					return false;
				}

				return true;
			}

			uint32_t GetSequence() const {
				return sequence;
			}

			bool IsComplete() const {
				return receivedFragments == numFragments;
			}

			const UdpEndpoint & GetEndpoint() const {
				return endpoint;
			}

			NETCODE_CONSTRUCTORS_DEFAULT_MOVE(DefragCtx);

			void AddFragment(UdpFragment frag) {
				if(static_cast<uint32_t>(frag.fragmentData.fragmentIndex) >= numFragments) {
					return;
				}

				++receivedFragments;

				UdpFragment * f = fragments.get() + frag.fragmentData.fragmentIndex;

				if(f->data == nullptr) {
					*f = std::move(frag);
				}
			}
			
			DefragCtx(uint32_t sequence, const UdpEndpoint & ep, uint32_t nf) :
				sequence{ sequence }, endpoint{ ep }, createdAt{ SystemClock::LocalNow() }, fragments{ nullptr }, receivedFragments{ 0 }, numFragments{ nf } {
				defragTarget = std::make_shared<GamePacket>();
				defragTarget->SetDataSize(GamePacket::MAX_DATA_SIZE);
				fragments = std::make_unique<UdpFragment[]>(numFragments);

				for(uint32_t i = 0; i < numFragments; i++) {
					UdpFragment * f = fragments.get() + i;
					f->sequence = 0;
					f->dataOffset = 0;
					f->data = nullptr;
				}

				createdAt = SystemClock::LocalNow();
			}
		};

		std::vector<DefragCtx> pendingFragments;
		concurrency::call<int> timerCallback;
		concurrency::timer<int> timeoutTimer;
		concurrency::critical_section lock;

		void TimerTick() {
			if(pendingFragments.empty()) {
				return;
			}
			
			concurrency::critical_section::scoped_lock scopedLock{ lock };

			Timestamp current = SystemClock::LocalNow();

			auto it = std::remove_if(std::begin(pendingFragments), std::end(pendingFragments), [&current](const DefragCtx & dc)->bool {
				return (dc.GetTimestamp() - current) > std::chrono::seconds(1);
			});

			if(it != std::end(pendingFragments)) {
				pendingFragments.erase(it, std::end(pendingFragments));
			}
		}
		
	public:
		DefragmenterAgent(ConnectionStorage * storage, concurrency::ISource<UdpFragment> & fragSource, Ptr<WorkToken> wt) :
			AgentBase{ wt }, connStorage{ storage }, fragmentSource{ fragSource }, pendingFragments{},
			timerCallback{ [this](int) -> void { TimerTick(); } }, timeoutTimer{ 1000, 0, &timerCallback, false }, lock{} { }

	protected:
		virtual void RunImpl() override {
			UdpFragment frag = concurrency::receive(fragmentSource);
			
			Ref<ConnectionBase> conn = connStorage->GetConnectionByEndpoint(frag.data->GetEndpoint());

			if(conn == nullptr) {
				Log::Debug("Connection was not found for the fragment, dropping packet");
				return;
				/*conn = std::make_shared<ConnectionBase>();
				conn->state = ConnectionState::ESTABLISHED;
				conn->remoteSequence = frag.sequence;
				conn->endpoint = frag.data->GetEndpoint();
				connStorage->AddConnection(conn);*/
			}

			if(conn->state != ConnectionState::ESTABLISHED) {
				Log::Debug("Connection is not established yet");
				return;
			}

			if(conn->remoteSequence > (frag.sequence + 5)) {
				Log::Debug("Received an old fragment, dropping packet");
				return;
			}

			conn->remoteSequence = std::max(conn->remoteSequence, frag.sequence);

			concurrency::critical_section::scoped_lock scopedLock{ lock };

			auto it = std::find_if(std::begin(pendingFragments), std::end(pendingFragments), [&frag](const DefragCtx & dc) -> bool {
				return frag.sequence == dc.GetSequence() && frag.data->GetEndpoint() == dc.GetEndpoint();
			});

			if(it == std::end(pendingFragments)) {
				DefragCtx dc{ frag.sequence, frag.data->GetEndpoint(), frag.fragmentData.fragmentCount };
				dc.AddFragment(std::move(frag));

				if(dc.IsComplete()) {
					Protocol::Update u;
					if(dc.TryDefragment(u)) {
						conn->sharedQueue.Received(std::move(u));
					}
				} else {
					pendingFragments.emplace_back(std::move(dc));
				}
			} else {
				it->AddFragment(std::move(frag));

				if(it->IsComplete()) {
					Protocol::Update u;
					if(it->TryDefragment(u)) {
						conn->sharedQueue.Received(std::move(u));
					}
					pendingFragments.erase(it);
				}
			}
		}
	};
	
	class NetcodeService {
	public:
#if defined(NETCODE_DEBUG)
		// basic socket with a debug readerwriter that enables fakelag
		using NetcodeSocketType = BasicSocket<boost::asio::ip::udp::socket, SyncAsioSocketHandler<boost::asio::ip::udp::socket>>;
#else
		using NetcodeSocketType = SharedUdpSocket;
#endif
		
	private:
		NetcodeSocketType socket;
		
		Ref<PacketStorage<UdpPacket>> packetStorage;
		
		Ref<PacketStorage<GamePacket>> gamePacketStorage;

		Ref<ManagedPool<FragmentationContext>> fragContextPool;

		ConnectionStorage connectionStorage;

		concurrency::unbounded_buffer<UdpAck> receivedAckBuffer;

		concurrency::unbounded_buffer<AckHandle> waitingForAckBuffer;

		concurrency::unbounded_buffer<SocketWriteHandle> writeRequests;

		concurrency::unbounded_buffer<ReliableUdpHandle> reliableHandles;

		concurrency::unbounded_buffer<GameMessageHandle> gameMessageHandles;

		concurrency::unbounded_buffer<UdpFragment> fragmentBuffer;

		concurrency::unbounded_buffer<Ref<UdpPacket>> receivedPackets;

		WorkToken workToken;

		ProtocolConfig protocolConfig;

		uint32_t mtu;

		UdpAckAgent ackAgent;

		ReliableUdpAgent reliableUdpAgent;

		UdpFragmenterAgent fragmenterAgent;

		UdpSocketWriterAgent<NetcodeSocketType> writerAgent;

		UdpSocketReaderAgent<NetcodeSocketType> readerAgent;

		ParserAgent parserAgent;

		DefragmenterAgent defragAgent;
		
	public:
		ConnectionStorage* GetConnections() {
			return &connectionStorage;
		}
		
		NetcodeService(NetcodeSocketType::SocketType sock) : socket { std::move(sock) },
			packetStorage { std::make_shared<PacketStorage<UdpPacket>>(16) },
			gamePacketStorage{ std::make_shared<PacketStorage<GamePacket>>(4) },
			fragContextPool{ std::make_shared<ManagedPool<FragmentationContext>>() },
			receivedAckBuffer{},
			waitingForAckBuffer{},
			writeRequests{},
			reliableHandles{},
			gameMessageHandles{},
			workToken{},
			protocolConfig{},
			mtu{ 1280 },
			ackAgent{ waitingForAckBuffer, receivedAckBuffer, &workToken },
			reliableUdpAgent{ reliableHandles, writeRequests, waitingForAckBuffer, receivedAckBuffer, &workToken },
			fragmenterAgent{ gameMessageHandles, writeRequests, &workToken },
			writerAgent{ writeRequests, &socket, &workToken },
			readerAgent{ &socket, packetStorage, receivedPackets, &workToken },
			parserAgent{ packetStorage, receivedPackets, receivedAckBuffer, writeRequests, fragmentBuffer, &workToken },
			defragAgent{ &connectionStorage, fragmentBuffer, &workToken } {

			ackAgent.start();
			reliableUdpAgent.start();
			fragmenterAgent.start();
			writerAgent.start();
			readerAgent.start();
			parserAgent.start();
			defragAgent.start();
			
		}
		
		concurrency::task<TrResult> Send(Protocol::Header headerOnlyMessage, const UdpEndpoint & endpoint, ResendArgs args) {
			if(headerOnlyMessage.type() == Protocol::MessageType::GAME) {
				throw UndefinedBehaviourException{ "bad API call" };
			}

			uint32_t seq = headerOnlyMessage.sequence();
			
			Ref<UdpPacket> pkt = packetStorage->GetBuffer();
			pkt->SetDataSize(UdpPacket::MAX_DATA_SIZE);
			pkt->SetEndpoint(endpoint);

			const uint32_t serializedHeaderSize = static_cast<uint32_t>(headerOnlyMessage.ByteSizeLong());
			const uint32_t totalHeaderSize = serializedHeaderSize + 8;

			google::protobuf::io::ArrayOutputStream outStream{ pkt->GetData(), static_cast<int32_t>(pkt->GetDataSize()) };
			google::protobuf::io::CodedOutputStream codedOutStream{ &outStream };

			codedOutStream.WriteLittleEndian32(static_cast<uint32_t>(serializedHeaderSize));
			if(!headerOnlyMessage.SerializeToCodedStream(&codedOutStream)) {
				Log::Warn("Failed to serialize header");
			}
			codedOutStream.WriteLittleEndian32(FragmentData{}.Pack());
			pkt->SetDataSize(totalHeaderSize);

			if((headerOnlyMessage.type() & 0x1) == 0x1) {
				ReliableUdpHandle handle;
				handle.data = std::move(pkt);
				handle.sequence = seq;
				handle.resendArgs = args;

				concurrency::send(reliableHandles, handle);

				return concurrency::task<TrResult>{ handle.completionEvent };
			} else {
				SocketWriteHandle swh;
				swh.endpoint = endpoint;
				swh.buffers[0] = pkt->GetConstBuffer();

				concurrency::send(writeRequests, swh);

				return concurrency::create_task(swh.completionEvent).then([pkt, seq](SocketOperationResult sor) -> TrResult {
					TrResult tr;
					tr.dataSize = pkt->GetDataSize();
					tr.attempts = 1;
					tr.fragments = 1;
					tr.sequence = seq;
					tr.sentBytes = sor.numBytes;
					tr.errorIfAny = sor.errorCode;
					tr.state = (sor.errorCode) ? TransmissionState::ERROR_WHILE_SENDING : TransmissionState::SUCCESS;
					return tr;
				});
			}
		}
		
		concurrency::task<TrResult> Send(Protocol::Header headerOnlyMessage, const UdpEndpoint& endpoint) {
			const auto args = protocolConfig.GetArgsFor(headerOnlyMessage.type());
			return Send(std::move(headerOnlyMessage), endpoint, args);
		}

		concurrency::task<TrResult> Send(uint32_t seq, Protocol::Update update, const UdpEndpoint& endpoint) {
			auto gm = gamePacketStorage->GetBuffer();
			gm->SetDataSize(GamePacket::MAX_DATA_SIZE);
			auto h = packetStorage->GetBuffer();
			h->SetDataSize(UdpPacket::MAX_DATA_SIZE);

			if(!update.SerializeToArray(gm->GetData(), static_cast<int>(gm->GetDataSize()))) {
				Log::Error("Failed to serialize game message");
				return concurrency::create_task([]() -> TrResult {
					TrResult tr;
					tr.state = TransmissionState::CANCELLED;
					tr.attempts = 0;
					tr.errorIfAny = Errc::make_error_code(Errc::result_out_of_range);
					return tr;
				});
			}

			gm->SetDataSize(static_cast<uint32_t>(update.ByteSizeLong()));
			gm->SetEndpoint(endpoint);
			h->SetEndpoint(endpoint);

			GameMessageHandle msg;
			msg.gamePacket = std::move(gm);
			msg.headerPacket = std::move(h);
			msg.sequence = seq;
			concurrency::send(gameMessageHandles, msg);

			return concurrency::create_task(msg.completionEvent);
		}
	};
	

}
