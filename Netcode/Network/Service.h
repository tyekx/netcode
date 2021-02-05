#pragma once

#include <NetcodeFoundation/Platform.h>
#include <Netcode/System/SystemClock.h>
#include <boost/asio.hpp>
#include "NetAllocator.h"
#include "Socket.hpp"
#include "MtuValue.hpp"
#include "SslUtil.h"
#include "Dtls.h"
#include "Connection.h"

#include <NetcodeProtocol/header.pb.h>

namespace Netcode::Network {

	struct ResendArgs {
		Duration resendInterval;
		uint32_t maxAttempts;

		constexpr ResendArgs() : resendInterval{}, maxAttempts{} {}

		constexpr ResendArgs(uint32_t resendIntervalMs, uint32_t numAttempts) :
			resendInterval{ std::chrono::milliseconds(resendIntervalMs) }, maxAttempts{ numAttempts } { }
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
				case Protocol::MessageType::CONNECT_DONE: return 12;
			}
		}

		constexpr static ResendArgs args[] = {
			ResendArgs{ 1000, 3 },
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
			ResendArgs{ 500, 5 },
		};
	public:

		ResendArgs GetArgsFor(Protocol::MessageType messageType) {
			return args[GetIndexOf(messageType)];
		}
	};

	/**
	 * Control message that has no connection associated with it
	 */
	struct NoAuthControlMessage : public ControlMessage {
		DtlsRoute * route;
	};

	class NetcodeService {
	public:
#if defined(NETCODE_DEBUG)
		using NetcodeSocketType = BasicSocket<boost::asio::ip::udp::socket, DebugAsioSocketReaderWriter<boost::asio::ip::udp::socket>>;
#else
		using NetcodeSocketType = SharedUdpSocket;
#endif

	private:
		boost::asio::io_context & ioContext;

		NetcodeSocketType socket;

		ConnectionStorage connectionStorage;

		PendingTokenStorage pendingTokenStorage;

		MessageQueue<NoAuthControlMessage> controlQueue;

		std::vector<std::unique_ptr<FilterBase>> filters;

		MtuValue linkLocalMtu;

		MtuValue mtu;

		ProtocolConfig protocolConfig;

		uint32_t receiveFailures;

		DtlsService dtls;
		
	public:
		boost::asio::io_context& GetIOContext() {
			return ioContext;
		}
		
		ConnectionStorage * GetConnections() {
			return &connectionStorage;
		}

		const std::vector<std::unique_ptr<FilterBase>> & GetFilters() const {
			return filters;
		}

		std::vector<std::unique_ptr<FilterBase>> & GetFilters() {
			return filters;
		}

		UdpEndpoint GetLocalEndpoint() const {
			return socket.GetSocket().local_endpoint();
		}

		DtlsService * GetDtls() {
			return &dtls;
		}

		void AddFilter(std::unique_ptr<FilterBase> filter) {
			filters.emplace_back(std::move(filter));
		}

		void CheckFilterCompletion(std::vector<std::unique_ptr<FilterBase>> & fltrs);
		
		void ApplyFilters(const std::vector<std::unique_ptr<FilterBase>> & filters, DtlsRoute* route, ControlMessage & cm);

		void RunFilters();

		NetcodeService(boost::asio::io_context & ioContext, NetcodeSocketType::SocketType sock, uint32_t linkLocalMtu, ssl_ptr<SSL_CTX> clientContext, ssl_ptr<SSL_CTX> serverContext) :
			ioContext{ ioContext },
			socket{ std::move(sock) },
			connectionStorage{},
			linkLocalMtu{ linkLocalMtu },
			mtu{ linkLocalMtu },
			protocolConfig{},
			receiveFailures{},
			dtls{ ioContext, std::move(clientContext), std::move(serverContext) } {

		}

		MtuValue GetLinkLocalMtu() const {
			return linkLocalMtu;
		}

		enum class ParseResult {
			FAILED, COMPLETED, TOOK_OWNERSHIP
		};

	private:
		ParseResult HandleRoutedMessage(NetAllocator * alloc, DtlsRoute * route, UdpPacket * pkt);
		
		ParseResult HandleAuthenticatedMessage(NetAllocator * alloc, Ref<ConnectionBase> conn, UdpPacket * pkt);

		/**
		 * @note pkt is used as a fallback if route is null
		 */
		Protocol::Control * ReceiveControl(NetAllocator * alloc, DtlsRoute* route, UdpPacket * pkt, ArrayView<uint8_t> source);
		
		/**
		 * @note invokes the other ReceiveControl with pkt->GetData(), pkt->GetSize() as the source view
		 */
		Protocol::Control * ReceiveControl(NetAllocator * alloc, DtlsRoute* route, UdpPacket * pkt) {
			return ReceiveControl(alloc, route, pkt, ArrayView<uint8_t>{ pkt->GetData(), pkt->GetSize() });
		}

		/**
		 * @note pkt is used as a fallback if route is null
		 */
		void SendAck(NetAllocator* alloc, DtlsRoute * route, UdpPacket * pkt, uint32_t seq);
		
	public:
		ParseResult TryParseMessage(NetAllocator * alloc, UdpPacket * pkt);

		friend class DtlsService;

		void Close() {
			boost::system::error_code ec;
			socket.GetSocket().close(ec);
		}

		void StartReceive(Ref<NetAllocator> alloc) {
			UdpPacket * pkt = alloc->MakeUdpPacket(Utility::Align<uint32_t, 512u>(linkLocalMtu.GetMtu() + 512u));

			socket.Receive(pkt->GetMutableBuffer(), pkt->GetEndpoint(), [this, pkt, al = std::move(alloc)](const ErrorCode & ec, size_t s) mutable -> void {
				if(ec) {
					al->Clear();
					if(receiveFailures++ < 5) {
						StartReceive(std::move(al));
					}
					return;
				}

				receiveFailures = 0;
				pkt->SetSize(s);
				pkt->SetTimestamp(SystemClock::LocalNow());

				if(TryParseMessage(al.get(), pkt) == ParseResult::TOOK_OWNERSHIP) {
					Host(); // start receiving with a new buffer
				} else {
					al->Clear(); // clear this bad boy and reuse buffer
					StartReceive(std::move(al));
				}
			});
		}

		void Host() {
			Ref<NetAllocator> alloc = MakeSmallAllocator();

			StartReceive(std::move(alloc));
		}

		Ref<NetAllocator> MakeAllocator(uint32_t blockSize) const {
			return std::make_shared<NetAllocator>(&ioContext, blockSize);
		}
		
		// Based on MTU
		Ref<NetAllocator> MakeSmallAllocator() const {
			// the largest possible packet + 512 bytes for management
			uint32_t blockSize = Utility::Align<uint32_t, 512u>(linkLocalMtu.GetMtu() + 512u);
			return std::make_shared<NetAllocator>(&ioContext, blockSize);
		}

		/**
		 * Main interface for dispatching DTLS messages
		 * @param allocator optional. Uses (new) as a fallback
		 * @param ct optional. Signals the result. Success means all packets were dispatched properly.
		 * @param endpoint the target endpoint
		 * @note the ssl_ptr<BIO> only decrements the reference counter for the BIO object. Call BIO_up_ref prior if you need to keep the memory alive
		 */
		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, const UdpEndpoint & endpoint, ssl_ptr<BIO> wbio, uint16_t mtu);

		/*
		 * by default it checks if controlMessage.connection is set
		 * if not, then it falls back to the endpoint in question
		 */
		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, const DtlsRoute* route, const ControlMessage & controlMessage, const UdpEndpoint & endpoint, MtuValue pmtu, ResendArgs args);

		CompletionToken<TrResult> Send(const ControlMessage & cMsg, const DtlsRoute * route) {
			// alloc, endpoint, ResendArgs is deducable, Mtu is given
			return Send(cMsg.allocator, cMsg.allocator->MakeCompletionToken<TrResult>(), route, cMsg, route->endpoint, MtuValue{ route->mtu }, protocolConfig.GetArgsFor(cMsg.control->type()));
		}


		CompletionToken<TrResult> Send(const GameMessage & gMsg, ConnectionBase * connection);

		//CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, Protocol::Update * update, ConnectionBase * connection, uint32_t seq);


		CompletionToken<TrResult> Send(Ref<NetAllocator> allocator, CompletionToken<TrResult> ct, const DtlsRoute * route, ssl_ptr<BIO> wbio) {
			return Send(std::move(allocator), std::move(ct), route->endpoint, std::move(wbio), route->mtu);
		}
	};
	
}
