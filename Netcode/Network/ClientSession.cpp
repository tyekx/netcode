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
		concurrency::task_completion_event<ErrorCode> tce;
	public:
		ClientConnectResponseFilter(uint32_t expectedSeq, const UdpEndpoint & expectedEp, concurrency::task_completion_event<ErrorCode> tce) :
			createdAt{ SystemClock::LocalNow() }, seq { expectedSeq }, endpoint{ expectedEp }, tce{ std::move(tce) } {
			state = FilterState::RUNNING;
		}

		bool CheckTimeout(Timestamp checkAt) override {
			if((checkAt - createdAt) > std::chrono::seconds(10)) {
				tce.set(make_error_code(Error::TIMEDOUT));
				return true;
			}
			return false;
		}
		
		FilterResult Run(Ptr<NetcodeService> service, Timestamp ts, ControlMessage& cm) override {
			if(cm.content.type() == Protocol::MessageType::CONNECT_RESPONSE) {
				if(!cm.content.has_connect_response()) {
					return FilterResult::IGNORED;
				}
				
				if(cm.source != endpoint && cm.content.sequence() != seq) {
					return FilterResult::IGNORED;
				}
			}

			tce.set(make_error_code(static_cast<Error>(cm.content.connect_response().error_code())));
			
			state = FilterState::COMPLETED;
			return FilterResult::CONSUMED;
		}
	};
	
	void ClientSession::SendConnectRequest(concurrency::task_completion_event<ErrorCode> tce) {
		Protocol::Header h;
		h.set_sequence(connection->localSequence++);
		h.set_type(Protocol::MessageType::CONNECT_REQUEST);
		Protocol::ConnectRequest * connReq = h.mutable_connect_request();
		connReq->set_type(Protocol::ConnectType::DIRECT);
		Protocol::Endpoint * privateEndpoint = connReq->mutable_private_endpoint();
		auto ep = service->GetLocalEndpoint();
		privateEndpoint->set_addr(ep.address().to_string());
		privateEndpoint->set_port(ep.port());

		uint32_t seq = h.sequence();

		Log::Debug("Private endpoint: {0}", ep);

		connection->state = ConnectionState::AUTHENTICATING;
		concurrency::task<TrResult> handle = service->Send(std::move(h), connection->endpoint);
		handle.then([this, seq, tce](TrResult tr) -> void {
			if(tr.state == TransmissionState::SUCCESS) {
				concurrency::task_completion_event<ErrorCode> reqTce;
				service->AddFilter(std::make_unique<ClientConnectResponseFilter>(seq, connection->endpoint, reqTce));

				concurrency::create_task(reqTce).then([this, tce](ErrorCode ec) mutable -> void {
					if(ec) {
						tce.set(ec);
					} else {
						SynchronizeToServerClock(std::move(tce));
					}
				});
			} else if(tr.state == TransmissionState::TIMEOUT) {
				Log::Debug("Conn request TIMEOUT");
				tce.set(make_error_code(Error::TIMEDOUT));
			} else if(tr.state == TransmissionState::ERROR_WHILE_SENDING) {
				Log::Debug("Conn request ERROR: {0}", tr.errorIfAny.message());
				tce.set(tr.errorIfAny);
			} else {
				tce.set(make_error_code(Error::BAD_API_CALL));
			}
		});
	}

	struct ClockSyncResult {
		double offset;
		double delay;
		ErrorCode errorCode;
	};

	class ClockSyncResponseFilter : public FilterBase {
		Timestamp createdAt;
		concurrency::task_completion_event<ClockSyncResult> tce;
		Ref<ConnectionBase> serverConnection;
		NtpClockFilter clockFilter;
		uint32_t numUpdates;
	public:
		ClockSyncResponseFilter(Ref<ConnectionBase> serverConnection, concurrency::task_completion_event<ClockSyncResult> tce) :
			createdAt{ SystemClock::LocalNow() }, tce{ std::move(tce) }, serverConnection{ std::move(serverConnection) }, clockFilter{} {
			state = FilterState::RUNNING;
			numUpdates = 0;
		}

		bool CheckTimeout(Timestamp checkAt) override {
			if((checkAt - createdAt) > std::chrono::seconds(10)) {
				state = FilterState::COMPLETED;
				ClockSyncResult csr;
				csr.errorCode = make_error_code(Error::TIMEDOUT);
				csr.delay = 0.0;
				csr.offset = 0.0;
				tce.set(std::move(csr));
				return true;
			}
			return false;
		}
		
		FilterResult Run(Ptr<NetcodeService> service, Timestamp timestamp, ControlMessage& cm) override {
			Protocol::Header & header = cm.content;
			
			if(header.type() == Protocol::MessageType::CLOCK_SYNC_RESPONSE) {
				if(cm.source != serverConnection->endpoint) {
					return FilterResult::IGNORED;
				}

				if(!header.has_time_sync()) {
					return FilterResult::IGNORED;
				}
			}

			header.mutable_time_sync()->set_client_resp_reception(ConvertTimestampToUInt64(cm.receivedAt));
			
			clockFilter.Update(header.time_sync());

			numUpdates++;

			if(numUpdates >= 8) {
				ClockSyncResult csr;
				csr.errorCode = make_error_code(Error::SUCCESS);
				csr.delay = clockFilter.GetDelay();
				csr.offset = clockFilter.GetOffset();
				tce.set(std::move(csr));
				state = FilterState::COMPLETED;
			}

			return FilterResult::CONSUMED;
		}
	};
	
	void ClientSession::SynchronizeToServerClock(concurrency::task_completion_event<ErrorCode> connTce) {
		concurrency::task_completion_event<ClockSyncResult> clockTce;

		connection->state = ConnectionState::SYNCHRONIZING;
		service->AddFilter(std::make_unique<ClockSyncResponseFilter>(connection, clockTce));

		concurrency::create_task(clockTce).then([this, connTce](ClockSyncResult res) -> void {
			if(res.errorCode) {
				Log::Debug("Error while synchronizing: {0}", res.errorCode.message());
				connection->state = ConnectionState::INACTIVE;
			} else {
				Log::Debug("Clock offset: {0}, clock delay: {1}", res.offset, res.delay);
				connection->state = ConnectionState::ESTABLISHED;
				connTce.set(make_error_code(Error::SUCCESS));
			}
		});
	}

	void ClientSession::Tick() {
		service->RunFilters();

		if(connection != nullptr) {
			if(connection->state == ConnectionState::SYNCHRONIZING) {
				Protocol::Header h;
				h.set_sequence(connection->localSequence++);
				h.set_type(Protocol::MessageType::CLOCK_SYNC_REQUEST);
				Protocol::TimeSync * ts = h.mutable_time_sync();
				ts->set_client_req_transmission(ConvertTimestampToUInt64(SystemClock::LocalNow()));
				service->Send(std::move(h), connection->endpoint);
			}
		}
	}

}
