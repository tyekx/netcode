#include "ClientSession.h"
#include "Macros.h"
#include <Netcode/Logger.h>
#include <Netcode/Config.h>
#include <Netcode/Utility.h>

namespace Netcode::Network {

	void ClientSession::OnTimerExpired(const ErrorCode & ec)
	{
		if(ec == boost::asio::error::operation_aborted) {
			Log::Info("[Net] [Client] Tick operation aborted");
			return;
		}

		if(ec) {
			Log::Error("[Net] [Client] Timer: {0}", ec.message());
		} else {
			Tick();
			InitTimer();
		}
	}

	void ClientSession::InitTimer()
	{
		timer.expires_from_now(boost::posix_time::milliseconds(Config::Get<uint32_t>(L"network.client.tickIntervalMs:u32")));
		timer.async_wait([this, lifetime = shared_from_this()](const ErrorCode& ec) -> void {
			OnTimerExpired(ec);
		});
	}


	void ClientSession::OnRead(Ref<UdpPacket> packet) {
		Protocol::ServerUpdate message;
		
		if(!message.ParseFromArray(packet->GetData(), packet->GetDataSize())) {
			Log::Debug("[Net] [Client] Failed to parse message from stream: {0}", packet->GetDataSize());
			return;
		}

		if(message.has_time_sync()) {
			message.mutable_time_sync()->set_client_resp_reception(ConvertTimestampToUInt64(SystemClock::LocalNow()));
		}

		if(connection->peerPacketSequenceId < message.id()) {
			connection->peerPacketSequenceId = message.id();
			gameQueue.Received(std::move(message));
		} else {
			Log::Debug("[Net] [Client] Dropping server update because it is old or duplicated");
		}
	}

	void ClientSession::InitRead() {
		Ref<UdpPacket> packet = packetStorage->GetBuffer();
		
		packet->SetEndpoint(UdpEndpoint{});
		packet->SetDataSize(PACKET_STORAGE_SIZE);

		connection->socket->async_receive_from(packet->GetMutableBuffer(), packet->GetEndpoint(), 
			[lifetime = shared_from_this(), this, packet](const ErrorCode & ec, size_t size) -> void {

			RETURN_AND_LOG_IF_ABORTED(ec, "[Net] [Client] Read operation aborted");

			if(ec) {
				Log::Error("[Net] [Client] Failed to read: {0}", ec.message());
			} else if(packet->GetEndpoint() != connection->liveEndpoint) {
				Log::Warn("[Net] [Client] Rejected package because the source is not the server");
			} else {
				//Log::Debug("[###] READ {0}: BYTES {1} PORT {2}", static_cast<void *>(packet.get()), static_cast<uint64_t>(size), static_cast<uint64_t>(packet->GetEndpoint().port()));
				packet->SetDataSize(size);
				packet->SetTimestamp(SystemClock::LocalNow());
				OnRead(packet);
				InitRead();
			}
		});
	}

	ClientSession::ClientSession(boost::asio::io_context & ioc) :
		ioContext{ ioc }, packetStorage{}, gameQueue{ }, timer{ ioc },
		resolver{ boost::asio::make_strand(ioc) }, connection{}, clockFilter{}, queryValueAddress{}, queryValuePort{}
	{
		packetStorage = std::make_shared<PacketStorage<UdpPacket>>();
	}

	void ClientSession::Start()
	{
	}

	void ClientSession::Update(Protocol::ClientUpdate message) {
		Ref<UdpPacket> packet = packetStorage->GetBuffer();

		message.set_id(connection->packetSequenceId++);
		
		if(!message.SerializeToArray(packet->GetData(), PACKET_STORAGE_SIZE)) {
			Log::Error("[Net] [Client] Failed to serialize to array");
			return;
		}

		packet->SetDataSize(message.ByteSizeLong());
		packet->SetEndpoint(connection->liveEndpoint);
		packet->SetTimestamp(SystemClock::LocalNow());
		
		connection->socket->async_send_to(packet->GetConstBuffer(), packet->GetEndpoint(), [this, packet, lifetime = shared_from_this()](const ErrorCode & ec, size_t size) -> void {
			if(ec) {
				Log::Error("[Net] [Client] Failed to send message: {0}", ec.message());
			} else {
				//Log::Debug("[###] WRITE {0}: BYTES {1} PORT {2}", static_cast<void *>(packet.get()), static_cast<uint64_t>(size), static_cast<uint64_t>(packet->GetEndpoint().port()));
			}
		});
	}
	
	void ClientSession::InitTimeSyncStep() {
		Protocol::ClientUpdate cu;
		cu.set_nonce(connection->nonce);
		Protocol::TimeSync * timeSync = cu.mutable_time_sync();
		timeSync->set_client_req_transmission(ConvertTimestampToUInt64(SystemClock::LocalNow()));
		Update(std::move(cu));
	}

	Ref<Connection> ClientSession::Connect(std::string address, uint16_t port, std::string nonce) {
		connection = std::make_shared<Connection>();
		connection->nonce = std::move(nonce);
		connection->packetSequenceId = 1;

		clockFilter = NtpClockFilter{};
		
		queryValuePort = std::to_string(port);
		queryValueAddress = std::move(address);

		resolver.async_resolve(queryValueAddress, queryValuePort, UdpResolver::address_configured, [lifetime = shared_from_this(), this](const ErrorCode &ec, const UdpResolver::results_type & results) -> void {
			RETURN_ON_ERROR(ec, "[Net] [Client] address resolution failed: {0}");

			RETURN_IF_AND_LOG_ERROR(results.empty(), "[Net] [Client] address resolution failed");

			connection->liveEndpoint = (*results.begin());

			ErrorCode errorCode;
			UdpSocket socket{ ioContext };
			
			socket.open(connection->liveEndpoint.protocol(), errorCode);
			RETURN_ON_ERROR(errorCode, "[Net] [Client] failed to bind open: {0}");
			
			socket.bind(UdpEndpoint{ connection->liveEndpoint.protocol(), Config::Get<uint16_t>(L"network.client.localPort:u16") }, errorCode);
			RETURN_ON_ERROR(errorCode, "[Net] [Client] failed to bind socket: {0}");

			Log::Debug("Resolution succeeded: {0}", connection->liveEndpoint.address().to_string());
			Log::Debug("Resolved port: {0}", static_cast<int>(connection->liveEndpoint.port()));

			connection->socket = std::make_unique<UdpSocket>(std::move(socket));
			connection->state = ConnectionState::CONNECTING;
			connection->peerPacketSequenceId = 0;
			
			InitTimeSyncStep();
			
			InitRead();

			InitTimer();

			Log::Info("[Net] [Client] Started on port {0}", Config::Get<uint16_t>(L"network.client.localPort:u16"));
		});

		return connection;
	}

	void ClientSession::SetError(const boost::system::error_code & ec)
	{
		timer.cancel();
	}

	void ClientSession::Tick()
	{
		std::vector<Protocol::ServerUpdate> updates;
		gameQueue.GetIncomingPackets(updates);

		for(const Protocol::ServerUpdate & message : updates) {
			clockFilter.Update(message.time_sync());
		}

		InitTimeSyncStep();
		//SendAll();
	}

	void ClientSession::OnMessageSent(const ErrorCode & ec, size_t size)
	{
		RETURN_AND_LOG_IF_ABORTED(ec, "[Net] [Client] Write operation aborted");

		if(ec) {
			Log::Error("[Net] [Client] Failed to send message: {0}", ec.message());
		} else {
			Log::Debug("[Net] [Client] Successfully sent {0} byte(s)", size);
		}
	}
	
}
