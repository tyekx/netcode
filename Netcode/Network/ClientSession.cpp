#include "ClientSession.h"
#include "Macros.h"
#include <Netcode/Logger.h>
#include <Netcode/Config.h>
#include <Netcode/Utility.h>
#include <NetcodeProtocol/header.pb.h>
#include "NetworkErrorCode.h"
#include "Service.h"

namespace Netcode::Network {

	class ClientConnectResponseFilter : public FilterBase {
		Timestamp createdAt;
		Ref<ConnectionBase> connection;
		CompletionToken<ErrorCode> filterToken;
	public:
		ClientConnectResponseFilter(Ref<ConnectionBase> connection,
			CompletionToken<ErrorCode> filterToken) :
			createdAt{ SystemClock::LocalNow() },
			connection{ std::move(connection) },
			filterToken{ std::move(filterToken) } {
			state = FilterState::RUNNING;
		}

		bool CheckTimeout(Timestamp checkAt) override {
			if((checkAt - createdAt) > std::chrono::seconds(10)) {
				filterToken->Set(make_error_code(NetworkErrc::RESPONSE_TIMEOUT));
				return true;
			}
			return false;
		}
		
		FilterResult Run(Ptr<NetcodeService> service, Ptr<DtlsRoute> route, Timestamp ts, ControlMessage& cm) override {
			if(route != connection->dtlsRoute) {
				return FilterResult::IGNORED;
			}

			if(cm.control->type() != Protocol::MessageType::CONNECT_RESPONSE) {
				return FilterResult::IGNORED;
			}

			if(!cm.control->has_connect_response()) {
				return FilterResult::IGNORED;
			}

			const Protocol::ConnectResponse * response = &cm.control->connect_response();

			if(response->error_code() != 0 || response->type() == Protocol::ConnectType::DIRECT) {
				filterToken->Set(make_error_code(static_cast<NetworkErrc>(response->error_code())));
				state = FilterState::COMPLETED;
				connection->id = response->player_id();
				service->GetConnections()->AddConnection(connection);
				return FilterResult::CONSUMED;
			}
			
			if(response->type() == Protocol::ConnectType::DEFERRED) {
				// we are deferred, gotta start connecting to the other endpoint.
				if(!response->has_public_endpoint()) {
					return FilterResult::IGNORED;
				}

				if(response->nonce().empty()) {
					return FilterResult::IGNORED;
				}

				const Protocol::Endpoint & pubEp = response->public_endpoint();
				const std::string & pubAddr = pubEp.addr();
				uint16_t pubPort = static_cast<uint16_t>(pubEp.port());

				UdpEndpoint publicEndpoint{
					boost::asio::ip::make_address(pubAddr),
					pubPort
				};

				connection->endpoint = publicEndpoint;
				connection->pmtu = MtuValue{ MtuValue::DEFAULT };
				connection->localControlSequence = 1;
				connection->localGameSequence = 1;
				connection->remoteControlSequence = 0;
				connection->remoteGameSequence = 0;
				connection->state = ConnectionState::CONNECTING;
				connection->secret.Store(Utility::ToWideString(response->nonce()), L"netcode-nonce");
				//TODO close DTLS route.
				state = FilterState::COMPLETED;
				return FilterResult::CONSUMED;
			}

			filterToken->Set(make_error_code(NetworkErrc::BAD_MESSAGE));
			state = FilterState::COMPLETED;
			return FilterResult::CONSUMED;
		}
	};

	struct ClockSyncResult {
		double offset;
		double delay;
		ErrorCode errorCode;
	};

	class ClockSyncResponseFilter : public FilterBase {
		CompletionToken<ClockSyncResult> completionToken;
		Timestamp createdAt;
		Ref<ConnectionBase> serverConnection;
		NtpClockFilter clockFilter;
		uint32_t numUpdates;
	public:
		ClockSyncResponseFilter(Ref<ConnectionBase> serverConnection, CompletionToken<ClockSyncResult> tce) :
			completionToken{ std::move(tce) }, createdAt{ SystemClock::LocalNow() },  serverConnection{ std::move(serverConnection) }, clockFilter{} {
			state = FilterState::RUNNING;
			numUpdates = 0;
		}

		bool CheckTimeout(Timestamp checkAt) override {
			if((checkAt - createdAt) > std::chrono::seconds(10)) {
				state = FilterState::COMPLETED;
				ClockSyncResult csr;
				csr.errorCode = make_error_code(NetworkErrc::RESPONSE_TIMEOUT);
				csr.delay = 0.0;
				csr.offset = 0.0;
				completionToken->Set(csr);
				return true;
			}
			return false;
		}
		
		FilterResult Run(Ptr<NetcodeService> service, Ptr<DtlsRoute> route, Timestamp timestamp, ControlMessage& cm) override {
			if(route != serverConnection->dtlsRoute) {
				return FilterResult::IGNORED;
			}
			
			/*
			Protocol::Header * header = cm.header;
			
			if(header->type() == Protocol::MessageType::CLOCK_SYNC_RESPONSE) {
				if(source != serverConnection->endpoint) {
					return FilterResult::IGNORED;
				}

				if(!header->has_time_sync()) {
					return FilterResult::IGNORED;
				}
			}
			header->mutable_time_sync()->set_client_resp_reception(ConvertTimestampToUInt64(cm.packet->GetTimestamp()));
			
			clockFilter.Update(header->time_sync());
			*/
			numUpdates++;

			if(numUpdates >= 8) {
				ClockSyncResult csr;
				csr.errorCode = make_error_code(NetworkErrc::SUCCESS);
				csr.delay = clockFilter.GetDelay();
				csr.offset = clockFilter.GetOffset();
				completionToken->Set(csr);
				state = FilterState::COMPLETED;
			}
			
			return FilterResult::CONSUMED;
		}
	};

	class PmtuDiscovery {
		constexpr static uint16_t COMMON_MTUS[] = {
			576,
			1006,
			1280,
			1472,
			1500,
			4352,
			8166,
			17914,
			65535
		};
	public:
		/**
		 * @param linkLocalMtu is an accumulator value in a recursive context
		 * @note result is saved into conn->pmtu
		 */
		static void Start(Ptr<NetcodeService> service, CompletionToken<ErrorCode> ct, Ref<ConnectionBase> conn, MtuValue linkLocalMtu) {
			/*
			* lets assume that we already established some communication with the host,
			* try sending the highest and work backwards
			*/
			Ref<NetAllocator> alloc = service->MakeSmallAllocator();
			Protocol::Control * h = alloc->MakeProto<Protocol::Control>();
			h->set_type(Protocol::MessageType::PMTU_DISCOVERY);
			h->set_sequence(conn->localControlSequence++);
			h->set_mtu_proble_size(linkLocalMtu.GetMtu());
			conn->pmtu = linkLocalMtu;

			ControlMessage cm;
			cm.allocator = alloc;
			cm.control = h;

			service ->Send(alloc, alloc->MakeCompletionToken<TrResult>(), nullptr, cm, conn->endpoint, conn->pmtu, ResendArgs{ 200, 3 })
					->Then([service, ct, conn, linkLocalMtu](const TrResult & tr) mutable -> void { 
				if(tr.errorCode) {
					uint32_t nextAttempt = 0;
					for(uint16_t i : COMMON_MTUS) {
						if(i >= linkLocalMtu.GetMtu()) {
							break;
						}
						nextAttempt = i;
					}

					if(nextAttempt == 0) {
						ct->Set(make_error_code(NetworkErrc::RESPONSE_TIMEOUT));
						return;
					}

					Start(service, std::move(ct), std::move(conn), MtuValue{ nextAttempt });
				} else {
					ct->Set(make_error_code(NetworkErrc::SUCCESS));
				}
			});
		}
	};

	CompletionToken<TrResult> ClientSession::StartPunchthrough()
	{
		Ref<NetAllocator> alloc = service->MakeAllocator(1024);
		Protocol::Control * control = alloc->MakeProto<Protocol::Control>();
		control->set_sequence(connection->localControlSequence++);
		control->set_type(Protocol::MessageType::CONNECT_PUNCHTHROUGH);

		ControlMessage cm;
		cm.allocator = alloc;
		cm.control = control;

		CompletionToken<TrResult> ct = alloc->MakeCompletionToken<TrResult>();
		
		return service->Send(alloc, ct, nullptr, cm, connection->endpoint, connection->pmtu, ResendArgs{ 1000, 10 });
	}

	CompletionToken<DtlsConnectResult> ClientSession::StartDtlsConnection()
	{
		Ref<NetAllocator> alloc = service->MakeAllocator(4096);
		return service->GetDtls()->InitConnect(service.get(), alloc.get(), connection->endpoint);
	}

	void ClientSession::SendConnectRequest(CompletionToken<ErrorCode> mainToken) {
		Ref<NetAllocator> alloc = service->MakeAllocator(2048);
		Protocol::Control * control = alloc->MakeProto<Protocol::Control>();
		control->set_sequence(connection->localControlSequence++);
		control->set_type(Protocol::MessageType::CONNECT_REQUEST);
		Protocol::ConnectRequest* cr = control->mutable_connect_request();
		cr->set_type(Protocol::ConnectType::DIRECT);
		cr->set_query("query");

		ControlMessage cm;
		cm.allocator = alloc;
		cm.control = control;

		CompletionToken<ErrorCode> ct = alloc->MakeCompletionToken<ErrorCode>();

		service->AddFilter(std::make_unique<ClientConnectResponseFilter>(connection, ct));

		service	->Send(alloc, alloc->MakeCompletionToken<TrResult>(), connection->dtlsRoute, cm, connection->endpoint, connection->pmtu, ResendArgs{ 1000, 5 })
				->Then([mainToken](const TrResult & result) {
				if(result.errorCode) {
					mainToken->Set(result.errorCode);
				}
			});

		ct->Then([mainToken](const ErrorCode & ec) {
			mainToken->Set(ec);
		});
	}
	
	void ClientSession::StartConnection(CompletionToken<ErrorCode> mainToken) {
		if(connection == nullptr) {
			Log::Error("No connection was set");
			mainToken->Set(make_error_code(NetworkErrc::BAD_REQUEST));
			return;
		}
		
		connection->pmtu = MtuValue{ MtuValue::DEFAULT };
		connection->state = ConnectionState::CONNECTING;

		StartPunchthrough()->Then([this, mainToken](const TrResult& result) {
			if(result.errorCode) {
				mainToken->Set(result.errorCode);
				return;
			}

			DiscoverPathMtu()->Then([this, mainToken](const ErrorCode & ec) {
				if(ec) {
					mainToken->Set(ec);
					return;
				}

				StartDtlsConnection()->Then([this, mainToken](const DtlsConnectResult & dcr) {
					if(dcr.errorCode) {
						mainToken->Set(dcr.errorCode);
						return;
					}

					DtlsRoute * route = dcr.route;

					SSL_SESSION * session = SSL_get_session(route->ssl.get());
					const SSL_CIPHER * cipher = SSL_SESSION_get0_cipher(session);
					char * ptr = SSL_CIPHER_description(cipher, NULL, 0);
					std::replace(ptr, ptr + strlen(ptr), '\n', '\0');
					Log::Debug("DTLS session established, used cipher: {0}", static_cast<const char*>(ptr));
					OPENSSL_free(ptr);
					
					connection->dtlsRoute = route;
					
					SendConnectRequest(mainToken);
				});
			});
		});
	}

	void ClientSession::OnHostnameResolved(const UdpResolver::results_type & results, CompletionToken<ErrorCode> mainToken) {

		if(results.empty()) {
			mainToken->Set(make_error_code(NetworkErrc::HOSTNAME_NOT_FOUND));
			return;
		}

		UdpSocket sock{ ioContext };

		ssl_ptr<SSL_CTX> clientCtx{ SSL_CTX_new(DTLSv1_2_client_method()) };

		SSL_CTX_set_options(clientCtx.get(), SSL_OP_NO_SSLv2 | SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_3 | SSL_OP_NO_COMPRESSION);
		
		if(SSL_CTX_set_cipher_list(clientCtx.get(), DTLS_CIPHERS) != 1) {
			Log::Error("Failed to set SSL cipher list");
		}

		connection->state = ConnectionState::CONNECTING;
		connection->endpoint = *(results.begin());
		connection->localControlSequence = 1;
		connection->localGameSequence = 1;
		connection->remoteControlSequence = 0;
		connection->remoteGameSequence = 0;

		boost::system::error_code ec;
		sock.open(connection->endpoint.protocol(), ec);

		if(ec) {
			Log::Error("Failed to open port");
			mainToken->Set(make_error_code(NetworkErrc::SOCK_ERROR));
			connection->state = ConnectionState::INACTIVE;
			return;
		}

		auto netInterfaces = GetCompatibleInterfaces(connection->endpoint.address());

		uint32_t linkLocalMtu = MtuValue::DEFAULT;

		if(netInterfaces.empty()) {
			Log::Warn("No defaultable network interface found");
		} else {
			const Interface bestCandidate = netInterfaces.front();

			sock.bind(UdpEndpoint{
				bestCandidate.address,
				0
				}, ec);

			if(ec) {
				Log::Error("Failed to bind port");
				mainToken->Set(make_error_code(NetworkErrc::SOCK_ERROR));
				connection->state = ConnectionState::INACTIVE;
				return;
			}

			linkLocalMtu = bestCandidate.mtu;

#if defined(NETCODE_DEBUG)
			try {
				uint32_t fakeMtu = Config::Get<uint32_t>(L"network.fakeMtu:u32");
				if(fakeMtu > 0) {
					linkLocalMtu = std::min(linkLocalMtu, fakeMtu);
				}
			} catch(OutOfRangeException &) { }
#endif
		}

		if(ec) {
			Log::Error("Failed to 'connect': {0}", ec.message());
			mainToken->Set(make_error_code(NetworkErrc::SOCK_ERROR));
			connection->state = ConnectionState::INACTIVE;
			return;
		}

		if(!SetDontFragmentBit(sock)) {
			Log::Error("Failed to set dont fragment bit");
			mainToken->Set(make_error_code(NetworkErrc::SOCK_ERROR));
			connection->state = ConnectionState::INACTIVE;
			return;
		}

		service = std::make_shared<NetcodeService>(ioContext, std::move(sock), static_cast<uint16_t>(linkLocalMtu), std::move(clientCtx), nullptr);
		service->Host();
		connection->tickInterval.store(std::chrono::milliseconds(250), std::memory_order_release);
		connection->tickCounter.store(0, std::memory_order_release);

		StartConnection(std::move(mainToken));

		InitTick();
	}

	CompletionToken<ErrorCode> ClientSession::StartHostnameResolution() {
		CompletionToken<ErrorCode> ct = std::make_shared<CompletionTokenType<ErrorCode>>(&ioContext);
		connection->state = ConnectionState::RESOLVING;
		resolver.async_resolve(queryValueAddress, queryValuePort, boost::asio::ip::resolver_base::address_configured,
			[this, c = ct](const ErrorCode & ec, UdpResolver::results_type results) mutable -> void {
			if(ec) {
				c->Set(make_error_code(NetworkErrc::HOSTNAME_NOT_FOUND));
			} else {
				OnHostnameResolved(results, std::move(c));
			}
		});
		return ct;
	}

	CompletionToken<ErrorCode> ClientSession::DiscoverPathMtu() {
		CompletionToken<ErrorCode> ct = std::make_shared<CompletionTokenType<ErrorCode>>(&ioContext);

		PmtuDiscovery::Start(service.get(), ct, connection, service->GetLinkLocalMtu());

		return ct;
	}

	void ClientSession::SendDebugFragmentedMessage() {
		Ref<NetAllocator> alloc = service->MakeAllocator(1 << 14);
		uint8_t * data = alloc->MakeArray<uint8_t>(5000);

		memset(data, 'A', 5000);
		memset(data + 1189, 'B', 1189);
		memset(data + 2 * 1189, 'C', 1189);
		memset(data + 3 * 1189, 'D', 1189);
		
		GameMessage gm;
		gm.sequence = connection->localGameSequence++;
		gm.allocator = std::move(alloc);
		gm.content = ArrayView<uint8_t>{ data, 5000 };

		service->Send(gm, connection.get());
	}

	CompletionToken<ErrorCode> ClientSession::Synchronize() {
		CompletionToken<ErrorCode> ct = std::make_shared<CompletionTokenType<ErrorCode>>(&ioContext);
		CompletionToken<ClockSyncResult> syncToken = std::make_shared<CompletionTokenType<ClockSyncResult>>(&ioContext);

		service->AddFilter(std::make_unique<ClockSyncResponseFilter>(connection, syncToken));

		syncToken->Then([this, ct](const ClockSyncResult & cr) -> void {
			if(cr.errorCode) {
				Log::Error("Failed to synchronize clock");
			} else {
				Log::Debug("Clock sync OK. Offset: {0} Delta: {1}", cr.offset, cr.delay);
			}
			ct->Set(cr.errorCode);
		});

		return ct;
	}

	void ClientSession::CloseService() {
		if(service != nullptr) {
			boost::system::error_code ec;
			tickTimer.cancel(ec);
			service->Close();
			Netcode::SleepFor(std::chrono::milliseconds(100));
			service.reset();
		}
	}

	void ClientSession::Tick() {
		service->RunFilters();

		if(connection != nullptr) {
			connection->tickCounter.fetch_add(1, std::memory_order_release);
			
			if(connection->state == ConnectionState::SYNCHRONIZING) {
				/*
				Ref<NetAllocator> alloc = service->MakeSmallAllocator();
				Protocol::Header * h = alloc->MakeProto<Protocol::Header>();
				h->set_type(Protocol::MessageType::CLOCK_SYNC_REQUEST);
				Protocol::TimeSync * ts = h->mutable_time_sync();
				ts->set_client_req_transmission(ConvertTimestampToUInt64(SystemClock::LocalNow()));
				service->Send(std::move(alloc), connection.get(), h);
				*/
			}
		}
	}

}
