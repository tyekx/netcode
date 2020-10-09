#include "ClientSession.h"
#include "Macros.h"
#include <Netcode/Logger.h>
#include <Netcode/Config.h>
#include <Netcode/Utility.h>
#include <NetcodeProtocol/header.pb.h>
#include <google/protobuf/io/coded_stream.h>

namespace Netcode::Network {

	class ClientConnectResponseFilter : public FilterBase {
		Timestamp createdAt;
		uint32_t seq;
		UdpEndpoint endpoint;
		CompletionToken<TrResult> xmitToken;
		CompletionToken<ErrorCode> filterToken;
	public:
		ClientConnectResponseFilter(uint32_t expectedSeq,
			const UdpEndpoint & expectedEp,
			CompletionToken<TrResult> xmitToken,
			CompletionToken<ErrorCode> filterToken) :
			createdAt{ SystemClock::LocalNow() }, seq{ expectedSeq }, endpoint{ expectedEp }, xmitToken{ std::move(xmitToken) }, filterToken{ std::move(filterToken) } {
			state = FilterState::RUNNING;
		}

		bool CheckTimeout(Timestamp checkAt) override {
			if((checkAt - createdAt) > std::chrono::seconds(10)) {
				filterToken->Set(make_error_code(NetworkErrc::RESPONSE_TIMEOUT));
				return true;
			}
			return false;
		}
		
		FilterResult Run(Ptr<NetcodeService> service, Timestamp ts, ControlMessage& cm) override {
			const Protocol::Header * header = cm.header;
			UdpEndpoint ep = cm.packet->GetEndpoint();

			if(cm.header->type() == Protocol::MessageType::CONNECT_RESPONSE) {
				if(!xmitToken->IsCompleted()) {
					return FilterResult::IGNORED;
				}

				if(!header->has_connect_response()) {
					return FilterResult::IGNORED;
				}
				
				if(ep != endpoint && header->sequence() != seq) {
					return FilterResult::IGNORED;
				}
			}

			filterToken->Set(make_error_code(static_cast<NetworkErrc>(header->connect_response().error_code())));

			state = FilterState::COMPLETED;
			return FilterResult::CONSUMED;
		}
	};
	
	void ClientSession::SendConnectRequest(CompletionToken<ErrorCode> mainToken) {
		Ref<NetAllocator> alloc = service->MakeSmallAllocator();
		Protocol::Header * header = alloc->MakeProto<Protocol::Header>();
		header->set_sequence(connection->localSequence++);
		header->set_type(Protocol::MessageType::CONNECT_REQUEST);
		Protocol::ConnectRequest * connReq = header->mutable_connect_request();
		connReq->set_type(Protocol::ConnectType::DIRECT);
		Protocol::Endpoint * privateEndpoint = connReq->mutable_private_endpoint();
		auto ep = service->GetLocalEndpoint();
		privateEndpoint->set_addr(ep.address().to_string());
		privateEndpoint->set_port(ep.port());

		uint32_t seq = header->sequence();

		CompletionToken<ErrorCode> filterCompletionToken = alloc->MakeCompletionToken<ErrorCode>();

		connection->state = ConnectionState::AUTHENTICATING;
		CompletionToken<TrResult> transmissionToken = service->Send(std::move(alloc), header, connection->endpoint);
		service->AddFilter(std::make_unique<ClientConnectResponseFilter>(seq, connection->endpoint, transmissionToken, filterCompletionToken));

		transmissionToken->Then([mainToken](const TrResult & tr) -> void {
			if(tr.errorCode) {
				mainToken->Set(tr.errorCode);
			}
		});

		filterCompletionToken->Then([mainToken](const ErrorCode & ec) -> void {
			mainToken->Set(ec);
		});
	}

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
		
		FilterResult Run(Ptr<NetcodeService> service, Timestamp timestamp, ControlMessage& cm) override {
			UdpEndpoint source = cm.packet->GetEndpoint();
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
		static void Start(Ptr<NetcodeService> service, CompletionToken<ErrorCode> ct, Ref<ConnectionBase> conn, MtuValue linkLocalMtu) {
			/*
			* lets assume that we already established some communication with the host,
			* try sending the highest and work backwards
			*/
			Ref<NetAllocator> alloc = service->MakeSmallAllocator();
			Protocol::Header * h = alloc->MakeProto<Protocol::Header>();
			h->set_sequence(conn->localSequence++);
			h->set_type(Protocol::MessageType::PMTU_DISCOVERY);
			h->set_mtu_probe_value(linkLocalMtu.GetMtu());
			conn->pmtu = linkLocalMtu;

			service->Send(std::move(alloc), conn.get(), h, ResendArgs{ 200, 3 })->Then([service, ct, conn, linkLocalMtu](const TrResult & tr) -> void { 
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

					PmtuDiscovery::Start(service, ct, conn, MtuValue{ nextAttempt });
				} else {
					ct->Set(make_error_code(NetworkErrc::SUCCESS));
				}
			});
		}
	};

	CompletionToken<ErrorCode> ClientSession::DiscoverPathMtu() {
		CompletionToken<ErrorCode> ct = std::make_shared<CompletionTokenType<ErrorCode>>(&ioContext);

		PmtuDiscovery::Start(service.get(), ct, connection, service->GetLinkLocalMtu());

		return ct;
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

	void ClientSession::Tick() {
		service->RunFilters();

		if(connection != nullptr) {
			if(connection->state == ConnectionState::SYNCHRONIZING) {
				Ref<NetAllocator> alloc = service->MakeSmallAllocator();
				Protocol::Header * h = alloc->MakeProto<Protocol::Header>();
				h->set_sequence(connection->localSequence++);
				h->set_type(Protocol::MessageType::CLOCK_SYNC_REQUEST);
				Protocol::TimeSync * ts = h->mutable_time_sync();
				ts->set_client_req_transmission(ConvertTimestampToUInt64(SystemClock::LocalNow()));
				service->Send(std::move(alloc), h, connection->endpoint);
			}
		}
	}

}
