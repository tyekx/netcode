#pragma once

#include "NetworkCommon.h"
#include <Netcode/System/SystemClock.h>
#include <NetcodeProtocol/header.pb.h>
#include <NetcodeProtocol/netcode.pb.h>
#include <Netcode/Utility.h>
#include <Netcode/Sync/SlimReadWriteLock.h>
#include <Netcode/Config.h>
#include "NetcodeFoundation/Enum.hpp"
#include <boost/lockfree/queue.hpp>
#include "NetAllocator.h"
#include "BasicPacket.hpp"
#include "MtuValue.hpp"
#include "Dtls.h"
#include "FragmentStorage.h"
#include "SslUtil.h"
#include <Netcode/System/SecureString.h>

namespace Netcode::Network {

	enum class ConnectionState : uint32_t {
		INACTIVE = 0,
		RESOLVING = 0x4000,
		CONNECTING = 0x8000,
		AUTHENTICATING = 0x8001,
		SYNCHRONIZING = 0x8002,
		ESTABLISHED = 0x10000
	};

	enum class AckClassification : uint32_t {
		EXTERNAL_INSECURE,
		EXTERNAL_SECURE,
		INTERNAL
	};

	NETCODE_ENUM_CLASS_OPERATORS(ConnectionState)
	
	using WaitableTimer = boost::asio::basic_waitable_timer<ClockType>;
	
	struct ControlMessage;

	struct GameMessage {
		
	};
	
	class ConnectionBase : public std::enable_shared_from_this<ConnectionBase> {
	public:
		std::atomic<Duration> tickInterval;
		std::atomic_uint32_t tickCounter;
		MtuValue pmtu;
		UdpEndpoint endpoint;
		Enum<ConnectionState> state;
		uint32_t localGameSequence;
		uint32_t localControlSequence;
		uint32_t remoteGameSequence;
		uint32_t remoteControlSequence;
		DtlsRoute * dtlsRoute;
		FragmentStorage fragmentStorage;
		boost::asio::strand<boost::asio::io_context::executor_type> strand;
		SecureString secret;
		MessageQueue<ControlMessage> sharedControlQueue;
		MessageQueue<GameMessage> sharedQueue;

		ConnectionBase(boost::asio::io_context& ioc) :
			tickInterval{},
			tickCounter{},
			pmtu{ MtuValue::DEFAULT },
			endpoint{},
			state{ ConnectionState::INACTIVE },
			localGameSequence{ 1 },
			localControlSequence{ 1 },
			remoteGameSequence{ 0 },
			remoteControlSequence{ 0 },
			dtlsRoute{ nullptr },
			fragmentStorage{},
			strand { make_strand(ioc) },
			sharedControlQueue{},
			sharedQueue{} { }
	};
	
	struct ControlMessage {
		Ref<NetAllocator> allocator;
		UdpPacket * packet; 
		Protocol::Control * control;

		ControlMessage() : allocator{}, packet{}, control{} { }
		~ControlMessage() noexcept = default;

		NETCODE_CONSTRUCTORS_DEFAULT_MOVE(ControlMessage);
		NETCODE_CONSTRUCTORS_DEFAULT_COPY(ControlMessage);
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
		FilterBase() : state { FilterState::IDLE } { }
		virtual ~FilterBase() = default;

		NETCODE_CONSTRUCTORS_DEFAULT_MOVE(FilterBase);
		NETCODE_CONSTRUCTORS_DELETE_COPY(FilterBase);
		
		virtual bool IsCompleted() const {
			return state == FilterState::COMPLETED;
		}

		virtual bool CheckTimeout(Timestamp checkAt) {
			return false;
		}

		virtual FilterResult Run(Ptr<NetcodeService> service, Ptr<DtlsRoute> route, Timestamp timestamp, ControlMessage & cm) {
			return FilterResult::IGNORED;
		}
	};

	struct PendingTokenNode {
		CompletionToken<TrResult> token;
		WaitableTimer * timer;
		UdpPacket * packet;
		PendingTokenNode * next;
		AckClassification ackClass;

		PendingTokenNode(CompletionToken<TrResult> token, WaitableTimer * timer, UdpPacket * packet, AckClassification classification) :
			token{ std::move(token) }, timer{ timer }, packet{ packet }, next{ nullptr }, ackClass{ classification } { }
	};

	class PendingTokenStorage {
		SlimReadWriteLock srwLock;
		PendingTokenNode * head;

	public:
		PendingTokenStorage() : srwLock{}, head{ nullptr }{}
		
		void Ack(uint32_t sequence, const UdpEndpoint & sender, AckClassification ackClass);

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
		ArrayView<uint8_t> content;
		UdpPacket * packet;
		const DtlsRoute * route;
		WaitableTimer * timer;
		Duration resendInterval;
		uint32_t attemptCount;
		uint32_t attemptIndex;

		using Base = std::enable_shared_from_this<ResendContext<SockType>>;

	public:
		ResendContext(PendingTokenStorage * pendingTokenStorage, 
			SockType * socket,
			CompletionToken<TrResult> token,
			ArrayView<uint8_t> content,
			UdpPacket * packet,
			const DtlsRoute * route,
			WaitableTimer * timer,
			Duration resendInterval,
			uint32_t numAttempts) : pendingTokenStorage{ pendingTokenStorage },
			socket{ socket },
			completionToken{ std::move(token) },
			content { content },
			packet{ packet },
			route { route },
			timer{ timer },
			resendInterval{ resendInterval },
			attemptCount{ numAttempts },
			attemptIndex{ 0 } {

		}

		void Attempt() {
			if(completionToken->IsCompleted()) {
				return;
			}
			
			if(attemptIndex == attemptCount) {
				completionToken->Set(TrResult{ make_error_code(NetworkErrc::RESEND_TIMEOUT), attemptCount * packet->GetSize() });
				return;
			}

			if(route != nullptr) {
				const ErrorCode ec = SslSend(route->ssl.get(), packet, content);
				if(ec) {
					completionToken->Set(TrResult{ ec });
					return;
				}
			} else {
				if(attemptIndex == 0) {
					packet->SetDataUnsafe(const_cast<uint8_t *>(content.Data()), content.Size());
					packet->SetSize(content.Size());
				}
			}

			attemptIndex++;

			Log::Debug("AttemptIndex:{0} AttemptCount: {1}", static_cast<int>(attemptIndex), static_cast<int>(attemptCount));
			
			socket->Send(packet->GetConstBuffer(), packet->GetEndpoint(),
				[this, lt = Base::shared_from_this()](const ErrorCode & ec, size_t s) -> void {
				if(ec) {
					completionToken->Set(TrResult{ make_error_code(NetworkErrc::SOCK_ERROR), (attemptIndex - 1) * packet->GetSize() });
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
					return;
				}

				/*
				 * ec is presumably a cancellation error, but check if the operation is completed,
				 * otherwise a memoryleak could arise.
				 * Concurrency here does not matter, 2 ACK on the same packet will be silently ignored
				 */
				if(!completionToken->IsCompleted()) {
					/*AckMessage ack;
					ack.endpoint = packet->GetEndpoint();
					ack.sequence = packet->GetSequence();
					pendingTokenStorage->Ack(ack);*/
				}
			});
		}
	};

}
