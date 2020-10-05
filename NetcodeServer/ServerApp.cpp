#include "ServerApp.h"

namespace nn = Netcode::Network;

class ServerClockSyncRequestFilter : public nn::FilterBase {
public:
	nn::FilterResult Run(Ptr<nn::NetcodeService> service, Netcode::Timestamp timestamp, nn::ControlMessage& cm) override {
		Netcode::Protocol::Header & header = cm.content;

		if(header.type() == Netcode::Protocol::MessageType::CLOCK_SYNC_REQUEST) {
			Netcode::Protocol::Header h;
			h.set_sequence(header.sequence());
			h.set_allocated_time_sync(header.release_time_sync());
			h.set_type(Netcode::Protocol::CLOCK_SYNC_RESPONSE);
 			Netcode::Protocol::TimeSync* ts = h.mutable_time_sync();
			ts->set_server_req_reception(Netcode::ConvertTimestampToUInt64(cm.receivedAt));
			ts->set_server_resp_transmission(Netcode::ConvertTimestampToUInt64(Netcode::SystemClock::LocalNow()));
			service->Send(std::move(h), cm.source);
			return nn::FilterResult::CONSUMED;
		}

		return nn::FilterResult::IGNORED;
	}
};

class ServerConnectRequestFilter : public nn::FilterBase {
	uint32_t maxConnections;
	
	void SendConnectResponse(Ptr<nn::NetcodeService> service, uint32_t seq, const nn::UdpEndpoint& target, int errorCode) {
		Netcode::Protocol::Header h;
		h.set_sequence(seq);
		h.set_type(Netcode::Protocol::MessageType::CONNECT_RESPONSE);
		Netcode::Protocol::ConnectResponse * cr = h.mutable_connect_response();
		cr->set_type(Netcode::Protocol::DIRECT);
		cr->set_error_code(errorCode);
		service->Send(std::move(h), target);
	}
	
public:
	ServerConnectRequestFilter() : maxConnections{ 10 } {
		state = Netcode::Network::FilterState::RUNNING;
	}
	
	virtual nn::FilterResult Run(Ptr<nn::NetcodeService> service, Netcode::Timestamp timestamp, nn::ControlMessage& cm) override {
		const Netcode::Protocol::Header & header = cm.content;
		nn::ConnectionStorage * cs = service->GetConnections();
		
		if(header.type() == Netcode::Protocol::MessageType::CONNECT_REQUEST) {
			if(cs->GetConnectionByEndpoint(cm.source) != nullptr) {
				SendConnectResponse(service, header.sequence(), cm.source, static_cast<int>(nn::Error::CONNECTION_ALREADY_EXISTS));
				return Netcode::Network::FilterResult::CONSUMED;
			}

			if(cs->GetConnectionCount() >= maxConnections) {
				SendConnectResponse(service, header.sequence(), cm.source, static_cast<int>(nn::Error::SERVER_IS_FULL));
			} else {
				Ref<nn::ConnectionBase> conn = std::make_shared<Netcode::Network::ConnectionBase>();
				conn->state = Netcode::Network::ConnectionState::SYNCHRONIZING;
				conn->remoteSequence = header.sequence();
				conn->localSequence = 1;
				conn->endpoint = cm.source;
				cs->AddConnection(std::move(conn));
				Log::Debug("Connection added");
				SendConnectResponse(service, header.sequence(), cm.source, static_cast<int>(nn::Error::SUCCESS));
			}
			
			return nn::FilterResult::CONSUMED;
		}
		
		return nn::FilterResult::IGNORED;
	}
};

void ServerApp::Setup(Netcode::Module::IModuleFactory * factory) {
	network = factory->CreateNetworkModule(this, 0);
	window = factory->CreateWindowModule(this, 1);
	
	StartModule(network.get());
	StartModule(window.get());

	auto ptr = network->CreateServer();
	serverSession = std::dynamic_pointer_cast<Netcode::Network::ServerSession>(ptr);
	serverSession->Start();
	service = serverSession->GetService();
	service->AddFilter(std::make_unique<ServerConnectRequestFilter>());
	service->AddFilter(std::make_unique<ServerClockSyncRequestFilter>());

	clientSession = network->CreateClient();
	clientSession->Start();
	//clientSession->Connect("::1", 9999, "");
}

/*
Advance simulation, update modules
*/

void ServerApp::Run() {
	uint32_t tickIntervalMs = Netcode::Config::Get<uint32_t>(L"network.server.tickIntervalMs:u32");

	Netcode::Duration frameDuration = std::chrono::milliseconds(tickIntervalMs);
	Netcode::Timestamp frameStartedAt = Netcode::SystemClock::LocalNow();
	
	while(window->KeepRunning()) {
		//serverSession->SwapBuffers(gameMessages);
		ProcessGameMessages();

		service->RunFilters();

		const Netcode::Timestamp frameFinishedAt = Netcode::SystemClock::LocalNow();
		const Netcode::Duration sim = frameFinishedAt - frameStartedAt;
		frameStartedAt = frameFinishedAt;

		
		
		if(sim < frameDuration) {
			Netcode::SleepFor(frameDuration - sim);
			/*
			Netcode::BusyWait(frameDuration - sim, [this]() -> void {
				window->ProcessMessages();
			});*/
		}
	}
}
