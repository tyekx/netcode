#pragma once

#include "NetworkCommon.h"
#include <Netcode/System/SystemClock.h>
#include <NetcodeProtocol/header.pb.h>
#include <NetcodeFoundation/Exceptions.h>
#include <Netcode/Utility.h>
#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/Config.h>
#include <random>

namespace Netcode::Network {

	enum class ConnectionState : uint32_t {
		INACTIVE,
		TIMEOUT,
		CONNECTING,
		SYNCHRONIZING,
		ESTABLISHED
	};
	
	using StreamPacket = BasicPacket<boost::asio::ip::udp, 65536>;
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

	template<typename T>
	class IndexTable {
		SlimReadWriteLock srwLock;
		std::vector<T> storage;

	public:
		IndexTable() : srwLock{}, storage{} {
			storage.reserve(32);
		}

		void Insert(T value) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			storage.emplace_back(std::move(value));
		}

		template<typename Query>
		T Find(Query && q) {
			ScopedSharedLock<SlimReadWriteLock> scopedLock{ srwLock };

			auto it = std::find_if(std::begin(storage), std::end(storage), q);

			if(it != std::end(storage)) {
				return *it;
			}

			return T{};
		}

		/*
		 * Query takes a const T& returns bool
		 */
		template<typename Query>
		T Erase(Query && q) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			auto it = std::find_if(std::begin(storage), std::end(storage), q);

			if(it != std::end(storage)) {
				std::swap(*it, storage.back());
				T tmp = std::move(storage.back());
				storage.pop_back();
				return tmp;
			}

			return T{};
		}

		template<typename Query>
		size_t EraseAll(Query && q) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			auto it = std::remove_if(std::begin(storage), std::end(storage), q);

			if(it != std::end(storage)) {
				size_t c = std::end(storage) - it;
				storage.erase(it, std::end(storage));
				return c;
			}

			return 0;
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


	enum class TransmissionResult : uint32_t {
		UNKNOWN,
		TIMEOUT,
		ERROR_WHILE_SENDING,
		SUCCESS
	};

	struct MessageTransmissionResult {
		TransmissionResult result;
		uint32_t sequenceId;
		uint32_t attempts;
		uint32_t fragments;
		uint32_t totalSentBytes;
		Timestamp transmissionStartedAt;
		Timestamp acknowledgeReceivedAt;
		ErrorCode errorIfAny;

		MessageTransmissionResult() : result{ TransmissionResult::UNKNOWN },
			sequenceId{},
			attempts{},
			fragments{},
			totalSentBytes{},
			transmissionStartedAt{},
			acknowledgeReceivedAt{},
			errorIfAny{} {}
	};


	class TransmissionHandleBase {
	protected:
		mutable SlimReadWriteLock srwLock;
		MessageTransmissionResult results;
		UdpEndpoint destEndpoint;
	public:
		TransmissionHandleBase(uint32_t sequenceId, const UdpEndpoint & dstEndpoint) : srwLock{}, results{}, destEndpoint{ dstEndpoint } {
			results.sequenceId = sequenceId;
		}

		bool IsComplete() const {
			return results.result != TransmissionResult::UNKNOWN;
		}

		[[nodiscard]]
		const UdpEndpoint & GetDestEndpoint() const {
			return destEndpoint;
		}

		/*
		 * Invoking this function while IsComplete() is false, is a racecondition
		 */
		[[nodiscard]]
		const MessageTransmissionResult & GetResult() const {
			return results;
		}
	};

	namespace Detail {

		struct ResendContext {
			Ref<UdpPacket> headerContent;
			Duration resendInterval;
			boost::asio::const_buffer constBuffer;

			ResendContext(Ref<UdpPacket> packet, uint32_t hdrSize, const Duration& resendInterval) :
				headerContent{ std::move(packet) }, resendInterval{ resendInterval }, constBuffer{ headerContent->GetData(), hdrSize } { }
		};

		struct FragmentationToken {};

		struct FragmentationContext {
			Ref<UdpPacket> headerContent;
			Ref<StreamPacket> messageContent;
			Ref<FragmentationToken> token;
			std::array<boost::asio::const_buffer, 2> constBuffers;

			uint32_t GetHeaderSize() const {
				return static_cast<uint32_t>(constBuffers[0].size());
			}

			uint32_t GetDataSize() const {
				return static_cast<uint32_t>(constBuffers[1].size());
			}

			FragmentationContext(Ref<UdpPacket> hdrContent, uint32_t hdrOffset, uint32_t hdrSize,
				Ref<StreamPacket> msgContent, uint32_t msgOffset, uint32_t msgSize,
				Ref<FragmentationToken> fragToken) :
				headerContent{ std::move(hdrContent) },
				messageContent{ std::move(msgContent) },
				token{ std::move(fragToken) },
				constBuffers{
					boost::asio::const_buffer{ headerContent->GetData() + hdrOffset, hdrSize },
					boost::asio::const_buffer{ messageContent->GetData() + msgOffset, msgSize }
			} { }
		};

		class RawUdpHandle : public TransmissionHandleBase {
			Ref<UdpPacket> headerContent;

		public:
			RawUdpHandle(Ref<UdpPacket> pkt, uint32_t seq, const UdpEndpoint & ep) :
				TransmissionHandleBase(seq, ep), headerContent{ std::move(pkt) } {}

			const UdpPacket * GetContent() const {
				return headerContent.get();
			}

			void Sent(const ErrorCode & ec, size_t size) {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

				if(ec) {
					results.result = TransmissionResult::ERROR_WHILE_SENDING;
					results.errorIfAny = ec;
				} else {
					results.result = TransmissionResult::SUCCESS;
					results.totalSentBytes = size;
				}
			}
		};

		class ReliableUdpHandle : public TransmissionHandleBase {
			WaitableTimer resendTimer;
			ResendContext resendCtx;
			Timestamp lastResentAt;

			void SetCompletionUnsafe(TransmissionResult result) {
				results.result = result;
			}

		public:
			ReliableUdpHandle(boost::asio::io_context & ioc, ResendContext resendCtx, uint32_t sequenceId, const UdpEndpoint & dstEndpoint) :
				TransmissionHandleBase{ sequenceId, dstEndpoint }, resendTimer{ ioc }, resendCtx{ std::move(resendCtx) }, lastResentAt{} {

			}

			// no lock needed because of the use-case
			WaitableTimer & GetTimer() {
				return resendTimer;
			}

			const ResendContext * GetResendContext() const {
				return &resendCtx;
			}

			void AttemptedSend(const ErrorCode & ec, size_t numBytes) {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

				// if a thread marked it completed, we ignore this attempt
				if(IsComplete()) {
					return;
				}

				results.attempts += 1;
				results.totalSentBytes += numBytes;
				if(ec) {
					results.errorIfAny = ec;
					results.acknowledgeReceivedAt = Timestamp{};
					results.result = TransmissionResult::ERROR_WHILE_SENDING;
				}
			}

			void SetCompletion(TransmissionResult result) {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

				resendTimer.cancel();

				if(IsComplete()) {
					return;
				}

				SetCompletionUnsafe(result);
			}

			uint32_t GetAttemptCount() const {
				return results.attempts;
			}

			uint32_t GetSequenceId() const {
				return results.sequenceId;
			}
		};

		class FragmentedUdpHandle : public TransmissionHandleBase {
			Weak<FragmentationToken> associatedToken;

			void SetCompletionUnsafe(TransmissionResult result) {
				results.result = result;
			}
		public:
			using FragCtxHandle = ManagedPool<FragmentationContext>::ObjectType;

			std::array<FragCtxHandle, 16> fragments;
			uint32_t numFragments;

			using TransmissionHandleBase::TransmissionHandleBase;

			void SetCompletionWithError(const ErrorCode & ec) {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

				if(IsComplete()) {
					return;
				}

				results.errorIfAny = ec;
				SetCompletionUnsafe(TransmissionResult::ERROR_WHILE_SENDING);
			}

			void SetCompletion(TransmissionResult tr) {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

				if(IsComplete()) {
					return;
				}

				SetCompletionUnsafe(tr);
			}

			Ref<FragmentationToken> GetToken() {
				if(!Utility::IsWeakRefEmpty(associatedToken)) {
					return associatedToken.lock();
				}

				return Ref<FragmentationToken>{
					new FragmentationToken{},
						[this](FragmentationToken * p) -> void {
						if(!IsComplete()) {
							SetCompletionUnsafe(TransmissionResult::SUCCESS);
						}
					}
				};
			}

			void FragmentSent(const ErrorCode & ec, size_t size) {
				ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

				if(IsComplete()) {
					return;
				}

				results.fragments += 1;
				results.totalSentBytes += size;
				if(ec) {
					results.errorIfAny = ec;
					SetCompletionUnsafe(TransmissionResult::ERROR_WHILE_SENDING);
				}
			}
		};

	}

	class DefragmentationContext {

		struct Fragment {
			uint32_t headerSize;
			FragmentData fragData;
			Ref<UdpPacket> packetIncludingHeader;
		};

		uint32_t sequence;
		uint16_t numFragments;
		uint16_t numReceivedFragments;
		std::array<Fragment, 16> fragments;

	public:
		DefragmentationContext(uint32_t seq, uint16_t numFragments) : sequence{ seq }, numFragments{ numFragments }, numReceivedFragments{ 0 }, fragments{} { }

		uint32_t GetSequence() const {
			return sequence;
		}

		void FragmentReceived(uint32_t headerSize, FragmentData fragData, Ref<UdpPacket> pkt) {
			if(fragData.fragmentIndex >= 16) {
				return;
			}

			Fragment & f = fragments[fragData.fragmentIndex];

			if(f.packetIncludingHeader == nullptr) {
				f.headerSize = headerSize;
				f.fragData = fragData;
				f.packetIncludingHeader = std::move(pkt);
				numReceivedFragments++;
			}
		}

		bool IsDefragmentable() const {
			return numFragments == numReceivedFragments;
		}

		bool DefragmentInto(StreamPacket * dstPacket) {
			if(dstPacket == nullptr) {
				return false;
			}

			if(!IsDefragmentable()) {
				return false;
			}

			uint8_t * const dstDataStartPtr = dstPacket->GetData();
			const uint32_t dstDataMaxBytes = dstPacket->GetDataSize();
			uint32_t dstDataOffset = 0;
			for(uint16_t i = 0; i < numFragments; i++) {
				const Fragment & f = fragments[i];
				const uint8_t * srcData = f.packetIncludingHeader->GetData();
				uint16_t sz = static_cast<uint16_t>(f.packetIncludingHeader->GetDataSize());

				if(sz < (f.fragData.sizeInBytes + f.headerSize)) {
					return false;
				}

				if(srcData == nullptr) {
					return false;
				}

				if((dstDataOffset + sz) > dstDataMaxBytes) {
					return false;
				}

				memcpy(dstDataStartPtr + dstDataOffset, srcData + f.headerSize, f.fragData.sizeInBytes);
				dstDataOffset += f.fragData.sizeInBytes;
			}

			return true;
		}
	};
	
	class NetcodeUdpSocket {
		UdpSocket socket;
		mutable SlimReadWriteLock srwLock;
		boost::asio::io_context & ioContext;

#if defined(NETCODE_DEBUG)
		Ref<ManagedPool<WaitableTimer>> fakeLagTimers;
		Duration fakeLagAvgDuration;
		std::default_random_engine randomEngine;
		std::normal_distribution<double> fakeLagDistribution;

		Duration SampleFakeLag() {
			double d = fakeLagDistribution(randomEngine);

			return std::chrono::duration_cast<Duration>(std::chrono::duration<double, std::milli>(d));
		}
#endif
		
	public:
		NetcodeUdpSocket(boost::asio::io_context & ioc, UdpSocket sock) : socket{ std::move(sock) }, srwLock{}, ioContext{ ioc } {
#if defined(NETCODE_DEBUG)
			fakeLagTimers = std::make_shared<ManagedPool<WaitableTimer>>();
			uint32_t fakeLagAvgDur = 0;
			uint32_t fakeLagSigma = 0;
			try {
				fakeLagAvgDur = Config::Get<uint32_t>(L"network.fakeLagAvg:u32");
				fakeLagSigma = Config::Get<uint32_t>(L"network.fakeLagSigma:u32");
			} catch(OutOfRangeException & e) {}

			fakeLagAvgDuration = std::chrono::duration<uint64_t, std::milli>(fakeLagAvgDur);

			double sigma = std::max(static_cast<double>(fakeLagSigma), 0.001);
			
			fakeLagDistribution = std::normal_distribution<double>(static_cast<double>(fakeLagAvgDur), sigma);
#endif
		}

		boost::asio::io_context & GetIOContext() {
			return ioContext;
		}

		NETCODE_CONSTRUCTORS_DELETE_MOVE(NetcodeUdpSocket);
		NETCODE_CONSTRUCTORS_DELETE_COPY(NetcodeUdpSocket);

		MtuValue GetLinkLocalMtu() {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };
			SOCKET nativeSocket = socket.native_handle();

			if(nativeSocket != (INVALID_SOCKET)) {
				uint32_t value = 0;
				int len = sizeof(uint32_t);
				int rv = getsockopt(nativeSocket, IPPROTO_IP, IP_MTU, reinterpret_cast<char *>(&value), &len);

				if(rv != -1) {
					return MtuValue{ value, socket.local_endpoint().address().is_v4() };
				}
			}

			throw UndefinedBehaviourException{ "Failed to query link local MTU" };
		}

		void Send(Ref<Detail::RawUdpHandle> handle) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			const UdpPacket * p = handle->GetContent();

#if defined(NETCODE_DEBUG)
			if(fakeLagAvgDuration > Duration{}) {
				auto fakeLagTimer = fakeLagTimers->Get(ioContext);

				fakeLagTimer->expires_from_now(SampleFakeLag());
				fakeLagTimer->async_wait([handle, p, this, lifetime = std::move(fakeLagTimer)](const ErrorCode & ec) -> void {
					socket.async_send_to(boost::asio::const_buffer{ p->GetData(), p->GetDataSize() },
						handle->GetDestEndpoint(), [handle](const ErrorCode & ec, size_t sz) -> void {
						handle->Sent(ec, sz);
					});
				});
			} else
#endif
			{
				socket.async_send_to(boost::asio::const_buffer{ p->GetData(), p->GetDataSize() },
					handle->GetDestEndpoint(), [handle](const ErrorCode & ec, size_t sz) -> void {
					handle->Sent(ec, sz);
				});
			}
		}

		void Send(Ref<Detail::ReliableUdpHandle> handle) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			const Detail::ResendContext * resendCtx = handle->GetResendContext();
			
			WaitableTimer & timer = handle->GetTimer();
			timer.expires_from_now(resendCtx->resendInterval);


			// [ expiry ]
			timer.async_wait([this, handle](const ErrorCode & errorCode) -> void {
				if(!errorCode) { // if the timer is not cancelled
					if(!handle->IsComplete()) { // if the handle still yields no result
						uint32_t numAttempts = handle->GetAttemptCount();

						if(numAttempts < 10) {
							Send(std::move(handle)); // restart everything
						} else {
							handle->SetCompletion(TransmissionResult::TIMEOUT);
						}
					}
				}
			});

#if defined(NETCODE_DEBUG)
			if(fakeLagAvgDuration > Duration{}) {
				auto fakeLagTimer = fakeLagTimers->Get(ioContext);

				fakeLagTimer->expires_from_now(SampleFakeLag());
				fakeLagTimer->async_wait([handle, resendCtx, this, lifetime = std::move(fakeLagTimer)](const ErrorCode & ec) -> void {
					socket.async_send_to(resendCtx->constBuffer, handle->GetDestEndpoint(), [handle](const ErrorCode & ec, size_t sz) -> void {
						Log::Debug("Sent: {0}", static_cast<int>(sz));
						handle->AttemptedSend(ec, sz);
					});
				});
			} else
#endif
			{
				socket.async_send_to(resendCtx->constBuffer, handle->GetDestEndpoint(), [handle](const ErrorCode & ec, size_t sz) -> void {
					Log::Debug("Sent: {0}", static_cast<int>(sz));
					handle->AttemptedSend(ec, sz);
				});
			}
		}

		void Send(Ref<Detail::FragmentedUdpHandle> handle) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

#if defined(NETCODE_DEBUG)
			if(fakeLagAvgDuration > Duration{}) {
				auto fakeLagTimer = fakeLagTimers->Get(ioContext);

				fakeLagTimer->expires_from_now(SampleFakeLag());

				for(uint32_t i = 0; i < handle->numFragments; i++) {
					auto fragCtx = std::move(handle->fragments[i]);
					fakeLagTimer->async_wait([handle, this, fctx{ std::move(fragCtx) }](const ErrorCode & ec) mutable -> void {
						socket.async_send_to(fctx->constBuffers, handle->GetDestEndpoint(),
							[handle, f{ std::move(fctx) }](const ErrorCode & ec, size_t sz) -> void {
							handle->FragmentSent(ec, sz);
						});
					});
				}
			} else
#endif
			{
				for(uint32_t i = 0; i < handle->numFragments; i++) {
					auto fragCtx = std::move(handle->fragments[i]);

					socket.async_send_to(fragCtx->constBuffers, handle->GetDestEndpoint(),
						[f = std::move(fragCtx), handle](const ErrorCode & ec, size_t sz) -> void {
						handle->FragmentSent(ec, sz);
					});
				}
			}
		}

		/*
		 * Callback takes an ErrorCode and a Ref<UdpPacket>
		 */
		template<typename Callback>
		void Receive(Ref<UdpPacket> pkt, Callback&& c) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

#if defined(NETCODE_DEBUG)
			if(fakeLagAvgDuration > Duration{}) {
				socket.async_receive_from(pkt->GetMutableBuffer(), pkt->GetEndpoint(), [this, pkt, cc = std::move(c)](const ErrorCode & ec, size_t sz) -> void {
					auto fakeLagTimer = fakeLagTimers->Get(ioContext);
					fakeLagTimer->expires_from_now(SampleFakeLag());
					fakeLagTimer->async_wait([pkt, callback = std::move(cc), errorCode = ec, sz, lifetime = std::move(fakeLagTimer)](const ErrorCode & timerEc) -> void {
						pkt->SetDataSize(sz);
						pkt->SetTimestamp(SystemClock::LocalNow());
						callback(errorCode, std::move(pkt));
					});
				});
			} else 
#endif
			{
				socket.async_receive_from(pkt->GetMutableBuffer(), pkt->GetEndpoint(), [pkt, callback = std::move(c)](const ErrorCode & ec, size_t sz) -> void {
					pkt->SetDataSize(sz);
					pkt->SetTimestamp(SystemClock::LocalNow());
					callback(ec, std::move(pkt));
				});
			}
		}
	};

	class ConnectionBase : public std::enable_shared_from_this<ConnectionBase> {
	public:
		MessageQueue<Protocol::Header> sharedControlQueue;
		// shared between service and consumer
		MessageQueue<Protocol::Update> sharedQueue;
		// index table for the service
		IndexTable<Ref<DefragmentationContext>> defragIndices;
		// must be kept unchanged for a connection
		UdpEndpoint endpoint;
		// handled by the service
		uint32_t localSequence;
		// handled by the service
		uint32_t remoteSequence;
		// state of the connection, handled by the consumer
		ConnectionState state;
	};

	class Request {
		UdpEndpoint sourceEndpoint;
		Protocol::Header header;

	public:
		NETCODE_CONSTRUCTORS_NO_COPY(Request);

		Request(const UdpEndpoint & ep, Protocol::Header h) : sourceEndpoint{ ep }, header{ std::move(h) } { }
	};

	class NetcodeService {
		Ref<NetcodeUdpSocket> udpSocket;

		Ref<PacketStorage<UdpPacket>> packetStorage;
		// messages that need to be handled further up
		MessageQueue<Request> controlMsgQueue;
		// messages that are successfully assembled game messages
		MessageQueue<Protocol::Update> gameMsgQueue;
		// global ACK table
		IndexTable<Ref<Detail::ReliableUdpHandle>> reliableUdpIndices;

		IndexTable<Ref<ConnectionBase>> connections;

		Ref<ManagedPool<Detail::FragmentationContext>> fragContextPool;

		Ref<PacketStorage<StreamPacket>> messageBufferStorage;

		uint32_t mtu;

	public:
		NetcodeService(Ref<NetcodeUdpSocket> sock) :
			udpSocket{ std::move(sock) },
			packetStorage{ std::make_shared<PacketStorage<UdpPacket>>(32) },
			controlMsgQueue{},
			gameMsgQueue{},
			reliableUdpIndices{},
			connections{},
			fragContextPool{ std::make_shared<ManagedPool<Detail::FragmentationContext>>() },
			messageBufferStorage{ std::make_shared<PacketStorage<StreamPacket>>(8) },
			mtu{ 1280 } {

		}

		struct MessageProcData {
			uint32_t headerSize;
			FragmentData fragData;
			Protocol::Header header;
			UdpEndpoint sourceEndpoint;
			Ref<UdpPacket> pkt;
		};

		void HandleRequest(MessageProcData & msg) {
			Request r{ msg.sourceEndpoint, std::move(msg.header) };
			controlMsgQueue.Received(std::move(r));
		}

		void HandleClockSyncRequest(const MessageProcData & msg) {
			if(!msg.header.has_time_sync()) {
				return;
			}

			Protocol::Header header;
			header.set_sequence(msg.header.sequence());
			header.set_type(Protocol::MessageType::CLOCK_SYNC_RESPONSE);
			auto * timeSync = header.mutable_time_sync();
			timeSync->set_client_req_transmission(msg.header.time_sync().client_req_transmission());
			timeSync->set_server_req_reception(ConvertTimestampToUInt64(msg.pkt->GetTimestamp()));
			timeSync->set_server_resp_transmission(ConvertTimestampToUInt64(SystemClock::LocalNow()));

			Send(std::move(header), msg.sourceEndpoint);
		}

		void HandleAckMessage(const MessageProcData & msg) {
			UdpEndpoint sourceEndpoint = msg.sourceEndpoint;
			uint32_t sourceSequence = msg.header.sequence();
			Ref<Detail::ReliableUdpHandle> udpHandle = reliableUdpIndices.Erase([&sourceEndpoint, sourceSequence](const Ref<Detail::ReliableUdpHandle> & r) -> bool {
				return r->GetDestEndpoint() == sourceEndpoint && r->GetSequenceId() == sourceSequence;
			});

			if(udpHandle != nullptr) {
				udpHandle->SetCompletion(TransmissionResult::SUCCESS);
			}
		}

		void HandleGameMessage(const MessageProcData & msg) {
			uint32_t sourceSequence = msg.header.sequence();

			// for a game message more validation is in order
			if(msg.fragData.sizeInBytes > UdpPacket::MAX_DATA_SIZE) {
				return;
			}

			if(msg.fragData.fragmentCount > 16) {
				return;
			}

			if(msg.fragData.fragmentIndex >= msg.fragData.fragmentCount) {
				return;
			}

			Ref<ConnectionBase> conn = connections.Find([&msg](const Ref<ConnectionBase> & c) -> bool {
				return c->endpoint == msg.sourceEndpoint;
			});

			if(conn != nullptr) {
				if(conn->remoteSequence < (sourceSequence + 2)) {
					conn->remoteSequence = std::max(conn->remoteSequence, sourceSequence);
					auto defragCtx = conn->defragIndices.Find([sourceSequence](const Ref<DefragmentationContext> & d) -> bool {
						return d->GetSequence() == sourceSequence;
					});

					if(defragCtx != nullptr) {
						defragCtx->FragmentReceived(msg.headerSize, msg.fragData, std::move(msg.pkt));
					}
				}
			}
		}

		void OnPacketReceivedAsync(Ref<UdpPacket> pkt) {
			google::protobuf::io::ArrayInputStream arrayInputStream{ pkt->GetData(), static_cast<int32_t>(pkt->GetDataSize()) };
			google::protobuf::io::CodedInputStream inputStream{ &arrayInputStream };

			uint32_t headerSize;
			if(!inputStream.ReadLittleEndian32(&headerSize)) {
				Log::Debug("bad headerSize");
				return;
			}

			Protocol::Header header;
			if(!header.ParseFromCodedStream(&inputStream)) {
				Log::Debug("bad header");
				return;
			}

			uint32_t packedFragmentData;
			if(!inputStream.ReadLittleEndian32(&packedFragmentData)) {
				packedFragmentData = 0;
			}

			Protocol::MessageType  msgType = header.type();

			// check if we need to send an ack
			if((static_cast<uint32_t>(msgType) & 0x1) == 0x1) {
				Protocol::Header h;
				h.set_type(Protocol::MessageType::ACKNOWLEDGE);
				h.set_sequence(header.sequence());
				Send(std::move(h), pkt->GetEndpoint());
			}

			MessageProcData procData;
			procData.sourceEndpoint = pkt->GetEndpoint();
			procData.headerSize = headerSize;
			procData.fragData.Unpack(packedFragmentData);
			procData.header = std::move(header);

			switch(msgType) {
				case Protocol::MessageType::ACKNOWLEDGE:
					HandleAckMessage(procData);
					break;

				case Protocol::MessageType::GAME:
					HandleGameMessage(procData);
					break;

				// ACK was enough for these
				case Protocol::MessageType::CONNECT_PUNCHTHROUGH:
				case Protocol::MessageType::PMTU_DISCOVERY:
				case Protocol::MessageType::HEARTBEAT: break;

				case Protocol::MessageType::CLOCK_SYNC_REQUEST:
					HandleClockSyncRequest(procData);
					break;

				// these must be added to a queue for further processing
				case Protocol::MessageType::CLOCK_SYNC_RESPONSE:
				case Protocol::MessageType::NAT_DISCOVERY_REQUEST:
				case Protocol::MessageType::NAT_DISCOVERY_RESPONSE:
				case Protocol::MessageType::CONNECT_REQUEST:
				case Protocol::MessageType::CONNECT_RESPONSE:
				case Protocol::MessageType::DISCONNECT:
				case Protocol::MessageType::DISCONNECT_NOTIFY:
				case Protocol::MessageType::REGISTER_HOST:
					HandleRequest(procData);
					break;

				default: break;
			}
		}

		void InitReceive() {
			udpSocket->Receive(packetStorage->GetBuffer(), [this](const ErrorCode & ec, Ref<UdpPacket> udpPacket) -> void {
				if(ec) {
					Log::Debug("Err: {0}", ec.message());
				} else {
					udpPacket->SetTimestamp(SystemClock::LocalNow());
					InitReceive();
					OnPacketReceivedAsync(std::move(udpPacket));
				}
			});
		}


	public:
		// sending header only messages either RAW or Reliable UDP
		Ref<TransmissionHandleBase> Send(Protocol::Header headerOnlyMessage, const UdpEndpoint & dstEndpoint) {
			if(headerOnlyMessage.type() == Protocol::MessageType::GAME) {
				return nullptr;
			}

			Ref<UdpPacket> headerContent = packetStorage->GetBuffer();

			const uint32_t serializedHeaderSize = static_cast<uint32_t>(headerOnlyMessage.ByteSizeLong());
			const uint32_t totalHeaderSize = serializedHeaderSize + 8;

			google::protobuf::io::ArrayOutputStream outStream{ headerContent->GetData(), static_cast<int32_t>(headerContent->GetDataSize()) };
			google::protobuf::io::CodedOutputStream codedOutStream{ &outStream };

			codedOutStream.WriteLittleEndian32(static_cast<uint32_t>(serializedHeaderSize));
			if(!headerOnlyMessage.SerializeToCodedStream(&codedOutStream)) {
				Log::Warn("Failed to serialize header");
			}
			codedOutStream.WriteLittleEndian32(FragmentData{}.Pack());

			headerContent->SetDataSize(totalHeaderSize);
			headerContent->SetEndpoint(dstEndpoint);
			
			if((static_cast<uint32_t>(headerOnlyMessage.type()) & 0x1) == 0x1) {
				constexpr Duration DEBUG_RESEND_INTERVAL = std::chrono::milliseconds(500);
				Detail::ResendContext resendContext{ std::move(headerContent), totalHeaderSize, DEBUG_RESEND_INTERVAL };

				auto handle = std::make_shared<Detail::ReliableUdpHandle>(udpSocket->GetIOContext(), std::move(resendContext), headerOnlyMessage.sequence(), dstEndpoint);

				reliableUdpIndices.Insert(handle);
				udpSocket->Send(handle);

				return handle;
			} else {
				auto rawHandle = std::make_shared<Detail::RawUdpHandle>(std::move(headerContent), headerOnlyMessage.sequence(), dstEndpoint);

				udpSocket->Send(rawHandle);

				return rawHandle;
			}
		}

		Ref<TransmissionHandleBase> Send(uint32_t seq, Protocol::Update update, const UdpEndpoint & dstEndpoint) {
			Protocol::Header header;
			header.set_type(Protocol::MessageType::GAME);
			header.set_sequence(seq);

			uint32_t totalDataSizeInBytes = update.ByteSizeLong();

			const uint32_t serializedHeaderSize = static_cast<uint32_t>(header.ByteSizeLong());
			const uint32_t totalHeaderSize = serializedHeaderSize + 8;
			// assert MTU > totalHeaderSize
			const uint32_t fragmentedDataSize = mtu - totalHeaderSize;
			const uint32_t numFragments = totalDataSizeInBytes / fragmentedDataSize +
				(std::min(totalDataSizeInBytes % fragmentedDataSize, 1u)); // always need the ceil, except if its an exact fit

			Ref<StreamPacket> messageContent = messageBufferStorage->GetBuffer();

			Ref<Detail::FragmentedUdpHandle> handle = std::make_shared<Detail::FragmentedUdpHandle>(header.sequence(), dstEndpoint);
			handle->numFragments = 0;

			if(!update.SerializeToArray(messageContent->GetData(), messageContent->GetDataSize())) {
				handle->SetCompletionWithError(Errc::make_error_code(Errc::bad_message));
				return handle;
			}

			Ref<Detail::FragmentationToken> fragToken = handle->GetToken();

			Ref<UdpPacket> headerContent = packetStorage->GetBuffer();

			google::protobuf::io::ArrayOutputStream outStream{ headerContent->GetData(), static_cast<int32_t>(headerContent->GetDataSize()) };
			google::protobuf::io::CodedOutputStream codedOutStream{ &outStream };

			uint32_t remainingSize = totalDataSizeInBytes;
			uint32_t dataOffset = 0;
			uint32_t headerOffset = 0;
			for(uint32_t i = 0; i < numFragments; i++) {
				uint32_t currentFragmentSize = std::min(remainingSize, fragmentedDataSize);

				FragmentData fragData;
				fragData.sizeInBytes = static_cast<uint16_t>(currentFragmentSize);
				fragData.fragmentCount = static_cast<uint8_t>(numFragments);
				fragData.fragmentIndex = static_cast<uint8_t>(i);

				codedOutStream.WriteLittleEndian32(serializedHeaderSize);
				if(!header.SerializeToCodedStream(&codedOutStream)) {
					return nullptr;
				}
				codedOutStream.WriteLittleEndian32(fragData.Pack());

				auto fragCtx = fragContextPool->Get(headerContent, headerOffset, totalHeaderSize,
					messageContent, dataOffset, currentFragmentSize,
					fragToken);

				handle->numFragments += 1;
				handle->fragments[i] = std::move(fragCtx);

				remainingSize -= currentFragmentSize;
				dataOffset += currentFragmentSize;
				headerOffset += totalHeaderSize;
			}

			udpSocket->Send(handle);

			return handle;
		}

		void Host(/* args */) {
			InitReceive();
		}

	};

}
