#pragma once

#include "NetworkCommon.h"
#include <Netcode/System/SystemClock.h>
#include <NetcodeProtocol/header.pb.h>
#include <NetcodeProtocol/netcode.pb.h>
#include <NetcodeFoundation/Exceptions.h>
#include <Netcode/Utility.h>
#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/Config.h>
#include "NetcodeFoundation/Enum.hpp"
#include <Netcode/System/System.h>
#include <boost/lockfree/queue.hpp>
#include "Socket.hpp"
#include "NetAllocator.h"
#include "BasicPacket.hpp"

namespace Netcode::Network {

	enum class ConnectionState : uint32_t {
		INACTIVE = 0,
		RESOLVING = 0x4000,
		CONNECTING = 0x8000,
		AUTHENTICATING = 0x8001,
		SYNCHRONIZING = 0x8002,
		ESTABLISHED = 0x10000
	};

	NETCODE_ENUM_CLASS_OPERATORS(ConnectionState)
	
	using WaitableTimer = boost::asio::basic_waitable_timer<ClockType>;

	struct MtuValue {
		constexpr static uint32_t UDP_HEADER_SIZE = 8;
		constexpr static uint32_t IPV4_HEADER_SIZE = 20;
		constexpr static uint32_t IPV6_HEADER_SIZE = 40;
		constexpr static uint32_t LOOPBACK_HEADER = 4;

		uint32_t mtu;
	public:
		MtuValue(const MtuValue & rhs) = default;
		MtuValue & operator=(const MtuValue & rhs) = default;

		MtuValue() : mtu{ 0 } { }

		explicit MtuValue(uint32_t v) : mtu{ v } {

		}

		uint32_t GetMtu() const {
			return mtu;
		}
		
		// value will still include UDP header size.
		uint32_t GetMaxPayloadSize(const IpAddress & peerOrHostAddr) const {
			if(peerOrHostAddr.is_v4()) {
				return mtu - IPV4_HEADER_SIZE;
			}

			if(peerOrHostAddr.is_v6()) {
				return mtu - IPV6_HEADER_SIZE;
			}
		}
	};

	struct ControlMessage {
		Ref<NetAllocator> allocator;
		UdpPacket * packet;
		Protocol::Header * header;
	};

	struct GameMessage {
		Ref<NetAllocator> allocator;
		Protocol::Update * update;
	};

	struct AckMessage {
		uint32_t sequence;
		UdpEndpoint endpoint;
	};

	class ConnectionBase : public std::enable_shared_from_this<ConnectionBase> {
	public:
		MessageQueue<ControlMessage> sharedControlQueue;
		// shared between service and consumer
		MessageQueue<GameMessage> sharedQueue;
		// must be kept unchanged for a connection
		UdpEndpoint endpoint;
		// handled by the service
		uint32_t localSequence;
		// handled by the service
		uint32_t remoteSequence;
		// state of the connection, handled by the consumer
		Enum<ConnectionState> state;
		// raw mtu value including ip and udp header sizes
		MtuValue pmtu;
	};

	class ConnectionStorage {
		mutable SlimReadWriteLock srwLock;
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

		uint32_t GetConnectionCount() const {
			ScopedSharedLock<SlimReadWriteLock> scopedLock{ srwLock };
			return static_cast<uint32_t>(connections.size());
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

		constexpr static ResendArgs args[] = {
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
	
	struct TrResult {
		size_t numBytes;
		ErrorCode errorCode;

		TrResult() : numBytes{ 0 }, errorCode{ } { }

		explicit TrResult(const ErrorCode & ec) : numBytes{ 0 }, errorCode{ ec } {

		}

		TrResult(const ErrorCode & ec, size_t s) : numBytes{ s }, errorCode{ ec } {

		}
	};

	enum class FilterResult {
		IGNORED,
		ONLY_READ,
		CONSUMED
	};

	enum class FilterState {
		IDLE,
		RUNNING,
		COMPLETED
	};

	class NetcodeService;
	
	class FilterBase {
	protected:
		FilterState state;
	public:
		virtual ~FilterBase() = default;
		
		virtual bool IsCompleted() const {
			return state == FilterState::COMPLETED;
		}

		virtual bool CheckTimeout(Timestamp checkAt) {
			return false;
		}

		virtual FilterResult Run(Ptr<NetcodeService> service, Timestamp timestamp, ControlMessage & cm) {
			return FilterResult::IGNORED;
		}
	};

	struct PendingTokenNode {
		CompletionToken<TrResult> token;
		WaitableTimer * timer;
		UdpPacket * packet;
		PendingTokenNode * next;

		PendingTokenNode(CompletionToken<TrResult> token, WaitableTimer * timer, UdpPacket * packet) :
			token{ std::move(token) }, timer{ timer }, packet{ packet }, next{ nullptr } { }
	};

	class PendingTokenStorage {
		SlimReadWriteLock srwLock;
		PendingTokenNode * head;

	public:
		PendingTokenStorage() : srwLock{}, head{ nullptr }{}

		void Ack(AckMessage ack) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			PendingTokenNode * prev = nullptr;

			for(PendingTokenNode * iter = head; iter != nullptr; iter = iter->next) {
				if(iter->packet->GetEndpoint() == ack.endpoint && iter->packet->GetSequence() == ack.sequence) {
					if(prev == nullptr) {
						head = iter->next;
					} else {
						prev = iter->next;
					}

					TrResult tr;
					iter->token->Set(TrResult{ make_error_code(Errc::success), iter->packet->GetSize() });
					iter->timer->cancel();
					CompletionToken<TrResult> tmpToken = std::move(iter->token);
				}

				prev = iter;
			}
		}

		void AddNode(PendingTokenNode * node) {
			ScopedExclusiveLock<SlimReadWriteLock> scopedLock{ srwLock };

			node->next = head;
			head = node;
		}
	};

	template<typename SockType>
	class ResendContext : public std::enable_shared_from_this<ResendContext<SockType>> {
		PendingTokenStorage * pendingTokenStorage;
		SockType * socket;
		CompletionToken<TrResult> completionToken;
		UdpPacket * packet;
		WaitableTimer * timer;
		Duration resendInterval;
		uint32_t attemptCount;
		uint32_t attemptIndex;

		using Base = std::enable_shared_from_this<ResendContext<SockType>>;

	public:
		ResendContext(PendingTokenStorage * pendingTokenStorage, 
			SockType * socket,
			CompletionToken<TrResult> token,
			UdpPacket * packet,
			WaitableTimer * timer,
			Duration resendInterval,
			uint32_t numAttempts) : pendingTokenStorage{ pendingTokenStorage },
			socket{ socket },
			completionToken{ std::move(token) },
			packet{ packet },
			timer{ timer },
			resendInterval{ resendInterval },
			attemptCount{ numAttempts },
			attemptIndex{ 0 } {

		}

		void Attempt() {
			if(completionToken->IsCompleted()) {
				return;
			}

			attemptIndex++;

			if(attemptIndex == attemptCount) {
				completionToken->Set(TrResult{ make_error_code(Error::TIMEDOUT), attemptCount * packet->GetSize() });
				return;
			}

			socket->Send(packet->GetConstBuffer(), packet->GetEndpoint(),
				[this, lt = Base::shared_from_this()](const ErrorCode & ec, size_t s) -> void {
				if(ec) {
					completionToken->Set(TrResult{ make_error_code(Error::SOCK_ERR), (attemptIndex - 1) * packet->GetSize() });
				} else {
					InitTimer();
				}
			});
		}

		void InitTimer() {
			timer->expires_after(resendInterval);
			timer->async_wait([this, lt = Base::shared_from_this()](const ErrorCode & ec) -> void {
				if(!ec) {
					Attempt();
				}

				/*
				 * ec is presumably a cancellation error, but check if the operation is completed,
				 * otherwise a memoryleak could arise.
				 * Concurrency here does not matter, 2 ACK on the same packet will be silently ignored
				 */
				if(!completionToken->IsCompleted()) {
					AckMessage ack;
					ack.endpoint = packet->GetEndpoint();
					ack.sequence = packet->GetSequence();
					pendingTokenStorage->Ack(std::move(ack));
				}
			});
		}
	};

	template<typename SockType>
	class FragmentationContext : public std::enable_shared_from_this<FragmentationContext<SockType>> {
		CompletionToken<TrResult> completionToken;
		SockType * socket;
		UdpPacket * packet;
		uint32_t maxDataSize;
		uint32_t dataStart;
		uint32_t headerStart;
		uint32_t dataSize;
		uint32_t headerSize;
		uint32_t dataOffset;
		uint32_t headerOffset;
		uint32_t sentDataSize;
		std::array<boost::asio::const_buffer, 2> currentFragment;

		using Base = std::enable_shared_from_this<FragmentationContext<SockType>>;

	public:
		FragmentationContext(CompletionToken<TrResult> token,
			SockType * socket,
			UdpPacket * pkt,
			uint32_t maxDataSizePerFragment,
			uint32_t dataStart,
			uint32_t headerStart,
			uint32_t dataSize,
			uint32_t headerSize) :
			completionToken{ std::move(token) },
			socket{ socket },
			packet{ pkt },
			maxDataSize{ maxDataSizePerFragment },
			dataStart{ dataStart },
			headerStart{ headerStart },
			dataSize{ dataSize },
			headerSize{ headerSize },
			dataOffset{ 0 },
			headerOffset{ 0 },
			sentDataSize{ 0 },
			currentFragment{} {

		}

		void SendFragment() {
			const uint32_t cDataSize = std::min(maxDataSize, dataSize - dataOffset);
			const uint32_t cDataOffset = dataStart + dataOffset;
			const uint32_t cHeaderSize = headerSize;
			const uint32_t cHeaderOffset = headerStart + headerOffset;

			if(cDataSize == 0) {
				completionToken->Set(TrResult{ make_error_code(Errc::success), sentDataSize });
				return;
			}

			dataOffset += cDataSize;
			headerOffset += cHeaderSize;

			currentFragment[0] = boost::asio::const_buffer{ packet->GetData() + cHeaderOffset, cHeaderSize };
			currentFragment[1] = boost::asio::const_buffer{ packet->GetData() + cDataOffset, cDataSize };

			socket->Send(currentFragment, packet->GetEndpoint(), [this, lt = Base::shared_from_this()](const ErrorCode & ec, size_t s) -> void {
				if(ec) {
					completionToken->Set(TrResult{ make_error_code(Error::SOCK_ERR), sentDataSize });
				} else {
					sentDataSize += static_cast<uint32_t>(s);
					SendFragment();
				}
			});
		}
	};


	class FragmentStorage {

		struct FSItem {
			Ref<NetAllocator> allocator;
			Protocol::Header * header;
			UdpPacket * packet;
			FragmentData fragmentData;
			uint16_t linkCount;
			uint16_t dataOffset;
			Timestamp receivedAt;
			FSItem * orderedNext;
			FSItem * next;
			FSItem * prev;

			FSItem(Ref<NetAllocator> alloc, Protocol::Header* header, UdpPacket *packet, FragmentData fragData, uint16_t dataOffset) :
				allocator{ std::move(alloc) }, header{ header }, packet{ packet },
				fragmentData{ fragData }, linkCount{ 1 }, dataOffset{ dataOffset }, receivedAt{ SystemClock::LocalNow() },
				orderedNext{ nullptr }, next{ nullptr }, prev{ nullptr } {}

			bool IsMatching(const FSItem & rhs) const {
				return	header->sequence() == rhs.header->sequence() &&
						packet->GetEndpoint() == rhs.packet->GetEndpoint() &&
						fragmentData.fragmentCount == rhs.fragmentData.fragmentCount;
			}

			bool operator==(const FSItem & rhs) const noexcept {
				return fragmentData.fragmentIndex == rhs.fragmentData.fragmentIndex;
			}

			bool operator<(const FSItem & rhs) const noexcept {
				return fragmentData.fragmentIndex < rhs.fragmentData.fragmentIndex;
			}
		};

		boost::asio::io_context * ioc;
		ConnectionStorage * connectionStorage;
		boost::lockfree::queue<FSItem *> inQueue;
		FSItem * orderedHead;
		FSItem * orderedTail;
		FSItem * deletableHead;
		std::atomic_uint32_t generation;

		void MarkDeletable(FSItem * node) {
			node->next = deletableHead;
			node->prev = nullptr;
			deletableHead = node;
		}

		void Erase(FSItem ** currentHead, FSItem ** currentTail, FSItem * node) {
			if(*currentHead == node) {
				*currentHead = node->next;
			}

			if(*currentTail == node) {
				*currentTail = node->prev;
			}

			FSItem * l = node->prev;
			FSItem * r = node->next;

			if(r != nullptr) {
				r->prev = l;
			}

			if(l != nullptr) {
				l->next = r;
			}
		}

		FSItem * DReplace(FSItem ** currentHead, FSItem ** currentTail, FSItem * currentItem, FSItem * newNode) {
			newNode->next = nullptr;
			newNode->prev = nullptr;

			if(*currentHead == currentItem) {
				newNode->next = currentItem->next;
				*currentHead = newNode;
			}

			if(*currentTail == currentItem) {
				newNode->prev = currentItem->prev;
				*currentTail = newNode;
			}

			if(currentItem->next != nullptr) {
				currentItem->next->prev = newNode;
			}

			if(currentItem->prev != nullptr) {
				currentItem->prev->next = newNode;
			}

			currentItem->next = nullptr;
			currentItem->prev = nullptr;

			return newNode;
		}

		void PushFront(FSItem ** currentHead, FSItem ** currentTail, FSItem * node) {
			if(*currentHead == nullptr) {
				*currentHead = node;
				*currentTail = node;
				return;
			}

			node->prev = nullptr;
			node->next = (*currentHead);
			(*currentHead)->prev = node;
			*currentHead = node;
		}

		void OrderedInsert(FSItem * item) {
			for(FSItem * iter = orderedHead; iter != nullptr; iter = iter->next) {
				if(iter->IsMatching(*item)) {
					if(*iter == *item) {
						return MarkDeletable(item);
					}

					if(*item < *iter) {
						DReplace(&orderedHead, &orderedTail, iter, item);
						// keep the old timestamp for the new head
						item->receivedAt = iter->receivedAt;
						item->orderedNext = iter;
						item->linkCount += iter->linkCount;
					} else {
						FSItem * p = iter;

						for(FSItem * i2 = iter->orderedNext; i2 != nullptr; i2 = i2->orderedNext) {
							if(*i2 == *item) {
								return MarkDeletable(item);
							}

							if(*p < *item && *item < *i2) {
								item->orderedNext = i2;
								break;
							}
							p = i2;
						}

						iter->linkCount += 1;
						p->orderedNext = item;
					}
					return;
				}
			}

			PushFront(&orderedHead, &orderedTail, item);
		}

		bool FragmentsAreConsistent(FSItem * ptr, uint32_t * dataSize) {
			int32_t prevIndex = -1;
			uint32_t expectedSize = ptr->fragmentData.sizeInBytes;
			*dataSize = 0;

			for(FSItem * it = ptr; it != nullptr; it = it->orderedNext) {
				int32_t currentIndex = it->fragmentData.fragmentIndex;

				if((currentIndex - prevIndex) != 1) {
					return false;
				}

				// size should never increase
				if(expectedSize < it->fragmentData.sizeInBytes) {
					return false;
				}

				// MTU should be constant for a message
				if(expectedSize != it->fragmentData.sizeInBytes) {
					// we allow the last one to fail this check, otherwise we delete
					if(it->orderedNext != nullptr) {
						return false;
					}
				}


				const size_t sourceSize = it->packet->GetSize();
				const size_t sourceOffset = it->dataOffset;
				const size_t sourceFragSize = it->fragmentData.sizeInBytes;

				if(sourceFragSize != (sourceSize - sourceOffset)) {
					return false;
				}

				*dataSize += it->fragmentData.sizeInBytes;

				prevIndex = currentIndex;
			}

			return true;
		}

		void TryReassemble(FSItem * p) {
			uint32_t dataSize = 0;
			if(FragmentsAreConsistent(p, &dataSize)) {
				uint32_t requiredSpace = Utility::Align<uint32_t, 512u>(dataSize + 512u);
				
				Ref<NetAllocator> dstAllocator;
				
				if(p->linkCount != 1) {
					dstAllocator = std::make_shared<NetAllocator>(ioc, requiredSpace);
				} else {
					dstAllocator = p->allocator;
				}

				MutableArrayView<uint8_t> reassembledBinary{ dstAllocator->MakeArray<uint8_t>(dataSize), dataSize };

				uint32_t dstOffset = 0;

				for(FSItem * iter = p; iter != nullptr; iter = iter->orderedNext) {
					const uint8_t * sourceData = iter->packet->GetData();
					const uint32_t sourceOffset = iter->dataOffset;
					const uint32_t sourceFragSize = iter->fragmentData.sizeInBytes;

					memcpy(reassembledBinary.Data() + dstOffset, sourceData + sourceOffset, sourceFragSize);

					dstOffset += sourceFragSize;
				}

				Protocol::Update * update = dstAllocator->MakeProto<Protocol::Update>();
				google::protobuf::io::ArrayInputStream ais{ reassembledBinary.Data(), static_cast<int32_t>(reassembledBinary.Size()) };

				if(update->ParseFromZeroCopyStream(&ais)) {
					Ref<ConnectionBase> conn = connectionStorage->GetConnectionByEndpoint(p->packet->GetEndpoint());

					if(conn != nullptr) {
						GameMessage msg;
						msg.allocator = std::move(dstAllocator);
						msg.update = update;
						conn->sharedQueue.Received(std::move(msg));
					}
				}
			}

			FSItem * it = p;
			while(it != nullptr) {
				FSItem * tmp = it->orderedNext;
				MarkDeletable(it);
				it = tmp;
			}
		}

		void DeleteFragments() {
			FSItem * it = deletableHead;
			while(it != nullptr) {
				FSItem * tmp = it->next;
				Ref<NetAllocator> alloc = std::move(it->allocator);
				alloc.reset();
				it = tmp;
			}
			deletableHead = nullptr;
		}

		void CheckFragments() {
			Timestamp tNow = SystemClock::LocalNow();
			for(FSItem * it = orderedHead; it != nullptr; it = it->next) {
				// 1 second old fragments are way too old
				if((tNow - it->receivedAt) > std::chrono::seconds(1)) {
					Erase(&orderedHead, &orderedTail, it);
					MarkDeletable(it);
				} else {
					if(it->linkCount == static_cast<uint32_t>(it->fragmentData.fragmentCount)) {
						Erase(&orderedHead, &orderedTail, it);
						TryReassemble(it);
					}
				}
			}
		}

		void RunDefragmentation() {
			do {
				uint32_t cg = generation.load(std::memory_order_acquire);

				FSItem * it = nullptr;
				while(inQueue.pop(it)) {
					OrderedInsert(it);
				}
				
				// traverses the ordered list for possible reorders
				CheckFragments();

				// deallocates finalized fragments
				DeleteFragments();

				// if we caught up, we reset
				if(generation.compare_exchange_strong(cg, 0, std::memory_order_release)) {
					return;
				}
			} while(true);
		}
	public:
		FragmentStorage(boost::asio::io_context * ioc, ConnectionStorage * connStorage) : ioc{ ioc }, connectionStorage{ connStorage },
			inQueue{1024}, orderedHead{ nullptr }, orderedTail{ nullptr }, deletableHead{ nullptr }, generation{ 0 } {

		}

		void AddFragment(Ref<NetAllocator> alloc, UdpPacket * pkt, Protocol::Header * header, FragmentData fd, uint16_t dataOffset) {
			Ref<ConnectionBase> conn = connectionStorage->GetConnectionByEndpoint(pkt->GetEndpoint());

			if(conn == nullptr) {
				return;
			}

			FSItem * node = alloc->Make<FSItem>(std::move(alloc), header, pkt, fd, dataOffset);

			if(!inQueue.push(node)) {
				node->allocator.reset();
				Log::Error("Queue is full");
				return;
			}

			if(generation.fetch_add(1) == 0) {
				boost::asio::post(*ioc, [this]() -> void {
					RunDefragmentation();
				});
			}
		}

	};

	class NetcodeService {
	public:
#if defined(NETCODE_DEBUG)
		using NetcodeSocketType = BasicSocket<boost::asio::ip::udp::socket, AsioSocketReadWriter<boost::asio::ip::udp::socket>>;
#else
		using NetcodeSocketType = SharedUdpSocket;
#endif
		
	private:
		boost::asio::io_context & ioContext;

		NetcodeSocketType socket;

		ConnectionStorage connectionStorage;

		FragmentStorage fragmentStorage;
		
		PendingTokenStorage pendingTokenStorage;

		MessageQueue<ControlMessage> controlQueue;

		ProtocolConfig protocolConfig;

		MtuValue linkLocalMtu;

		MtuValue mtu;

		std::vector<std::unique_ptr<FilterBase>> filters;
		
	public:
		ConnectionStorage* GetConnections() {
			return &connectionStorage;
		}

		UdpEndpoint GetLocalEndpoint() const {
			return socket.GetSocket().local_endpoint();
		}

		void AddFilter(std::unique_ptr<FilterBase> filter) {
			filters.emplace_back(std::move(filter));
		}

		void RunFilters() {
			std::vector<ControlMessage> controlMessages;

			controlQueue.GetIncomingPackets(controlMessages);

			Timestamp ts = SystemClock::LocalNow();

			for(ControlMessage & cm : controlMessages) {
				for(std::unique_ptr<FilterBase> & f : filters) {
					if(!f->IsCompleted()) {
						FilterResult r = f->Run(this, ts, cm);

						if(r == FilterResult::CONSUMED) {
							break;
						}
					}
				}
			}

			auto it = std::remove_if(std::begin(filters), std::end(filters), [ts](const std::unique_ptr<FilterBase> & f) -> bool {
				return f->IsCompleted() || f->CheckTimeout(ts);
			});

			filters.erase(it, std::end(filters));
		}
		
		NetcodeService(boost::asio::io_context & ioContext, NetcodeSocketType::SocketType sock, uint32_t linkLocalMtu) :
			ioContext{ ioContext },
			socket { std::move(sock) },
			connectionStorage{},
			fragmentStorage{ &ioContext, &connectionStorage },
			protocolConfig{},
			linkLocalMtu{ linkLocalMtu },
			mtu{ linkLocalMtu } {

		}

		MtuValue GetLinkLocalMtu() const {
			return linkLocalMtu;
		}

		void SendAck(NetAllocator * alloc, uint32_t seq, const UdpEndpoint & ep) {

			Protocol::Header* h = alloc->MakeProto<Protocol::Header>();
			h->set_sequence(seq);
			h->set_type(Protocol::MessageType::ACKNOWLEDGE);

			const uint32_t serializedHeaderSize = static_cast<uint32_t>(h->ByteSizeLong());
			const uint32_t binarySize = serializedHeaderSize + 2 * sizeof(google::protobuf::uint32);
			uint8_t * buffer = google::protobuf::Arena::CreateArray<uint8_t>(alloc->GetArena(), binarySize);

			google::protobuf::io::ArrayOutputStream outStream{ buffer, static_cast<int32_t>(binarySize) };
			google::protobuf::io::CodedOutputStream codedOutStream{ &outStream };

			codedOutStream.WriteLittleEndian32(static_cast<uint32_t>(serializedHeaderSize));
			if(!h->SerializeToCodedStream(&codedOutStream)) {
				Log::Error("Failed to serialize ACK header");
				return;
			}
			codedOutStream.WriteLittleEndian32(FragmentData{}.Pack());

			socket.Send(boost::asio::mutable_buffer{ buffer, binarySize }, ep, [al = alloc->shared_from_this()](const ErrorCode & ec, size_t s) -> void {
				if(ec) {
					Log::Error("Failed to send ack: {0}", ec.message());
				}
			});
		}

		bool TryParseMessage(NetAllocator* alloc, UdpPacket * pkt) {
			Protocol::Header* header = alloc->MakeProto<Protocol::Header>();
			
			google::protobuf::io::ArrayInputStream ais{ pkt->GetData(), static_cast<int32_t>(pkt->GetSize()) };
			google::protobuf::io::CodedInputStream cis{ &ais };

			uint32_t serializedHeaderSize = 0;
			if(!cis.ReadLittleEndian32(&serializedHeaderSize)) {
				return false;
			}

			auto limit = cis.PushLimit(static_cast<int32_t>(serializedHeaderSize));
			if(!header->ParseFromCodedStream(&cis)) {
				return false;
			}
			cis.PopLimit(limit);

			uint32_t packedFragData = 0;
			if(!cis.ReadLittleEndian32(&packedFragData)) {
				return false;
			}

			FragmentData fd;
			fd.Unpack(packedFragData);

			if(header->type() == Protocol::MessageType::ACKNOWLEDGE) {

				AckMessage ack;
				ack.endpoint = pkt->GetEndpoint();
				ack.sequence = header->sequence();
				pendingTokenStorage.Ack(std::move(ack));
				return false; // signal false error, we dont need the memory anymore

			}

			if(header->type() == Protocol::MessageType::GAME) {

				fragmentStorage.AddFragment(alloc->shared_from_this(), pkt, header, fd, static_cast<uint16_t>(cis.CurrentPosition()));
				return true;

			}

			if((header->type() & 0x1) == 0x1) {

				SendAck(alloc, header->sequence(), pkt->GetEndpoint());

			}

			ControlMessage cm;
			cm.allocator = alloc->shared_from_this();
			cm.header = header;
			cm.packet = pkt;
			controlQueue.Received(std::move(cm));

			return true;
		}

		void StartReceive(Ref<NetAllocator> alloc) {
			UdpPacket * pkt = alloc->MakeUdpPacket(Utility::Align<uint32_t, 512u>(linkLocalMtu.GetMtu() + 512u));

			socket.Receive(pkt->GetMutableBuffer(), pkt->GetEndpoint(), [this, pkt, al = std::move(alloc)](const ErrorCode & ec, size_t s) mutable -> void {
				if(ec) {
					al->Clear();
					StartReceive(std::move(al));
					return;
				}

				pkt->SetSize(s);
				pkt->SetTimestamp(SystemClock::LocalNow());
				if(!TryParseMessage(al.get(), pkt)) {
					al->Clear();
					StartReceive(std::move(al));
					return;
				}

				Host();
			});
		}

		void Host() {
			Ref<NetAllocator> alloc = MakeSmallAllocator();

			StartReceive(std::move(alloc));
		}

		// optimal for receiving messages or sending small messages
		Ref<NetAllocator> MakeSmallAllocator() {
			// the largest possible packet + 512 bytes for management
			uint32_t blockSize = Utility::Align<uint32_t, 512u>(linkLocalMtu.GetMtu() + 512u);
			return std::make_shared<NetAllocator>(&ioContext, blockSize);
		}

		// optimal for sending game updates
		Ref<NetAllocator> MakeLargeAllocator() {
			constexpr uint32_t blockSize = 0x21000;
			return std::make_shared<NetAllocator>(&ioContext, blockSize);
		}


		// main interface for sending control messages
		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, Protocol::Header * header, const UdpEndpoint & endpoint, MtuValue pmtu, ResendArgs args);

		// main interface for sending updates
		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, Protocol::Update * update, const UdpEndpoint & endpoint, MtuValue pmtu, uint32_t seq);

		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, Protocol::Header * header, const UdpEndpoint & endpoint) {
			return Send(allocator, allocator->MakeCompletionToken<TrResult>(), header, endpoint, mtu, protocolConfig.GetArgsFor(header->type()));
		}

		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, ConnectionBase * connectionBase, Protocol::Header * header) {
			return Send(allocator, allocator->MakeCompletionToken<TrResult>(), connectionBase, header);
		}

		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, ConnectionBase * connectionBase, Protocol::Update * update) {
			return Send(allocator, allocator->MakeCompletionToken<TrResult>(), connectionBase, update);
		}

		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, ConnectionBase * connectionBase, Protocol::Header * header, ResendArgs args) {
			header->set_sequence(connectionBase->localSequence++);
			return Send(allocator, allocator->MakeCompletionToken<TrResult>(), header, connectionBase->endpoint, connectionBase->pmtu, args);
		}

		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> premadeToken, ConnectionBase * connectionBase, Protocol::Header * header) {
			header->set_sequence(connectionBase->localSequence++);
			return Send(std::move(allocator), std::move(premadeToken), header, connectionBase->endpoint, connectionBase->pmtu, protocolConfig.GetArgsFor(header->type()));
		}

		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> premadeToken, ConnectionBase * connectionBase, Protocol::Header * header, ResendArgs args) {
			header->set_sequence(connectionBase->localSequence++);
			return Send(std::move(allocator), std::move(premadeToken), header, connectionBase->endpoint, connectionBase->pmtu, args);
		}

		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> premadeToken, ConnectionBase * connectionBase, Protocol::Update * update) {
			return Send(std::move(allocator), std::move(premadeToken), update, connectionBase->endpoint, connectionBase->pmtu, connectionBase->localSequence++);
		}
	};

}
