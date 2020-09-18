#include <boost/asio.hpp>

#include "ServerSession.h"
#include "Macros.h"
#include <Netcode/Utility.h>
#include <Netcode/Logger.h>
#include <Netcode/Config.h>



namespace Netcode::Network {

	

	ServerSession::ServerSession(boost::asio::io_context & ioc) :
		ioContext{ ioc },
		timer{ ioc },
		gameQueue{},
		storage{ std::make_shared<PacketStorage<UdpPacket>>() },
		db{},
		dbContext{}{

		UdpSocket gameSocket{ ioc };

		uint32_t gamePort = Config::Get<uint16_t>(L"network.server.gamePort:u16");

		std::string selfAddr = Utility::ToNarrowString(Config::Get<std::wstring>(L"network.server.selfAddress:string"));

		ErrorCode ec;
		boost::asio::ip::address addr = boost::asio::ip::address::from_string(selfAddr, ec);

		RETURN_ON_ERROR(ec, "[Network] [Server] invalid configuration value: {0}");
		
		ec = Bind(addr, gameSocket, gamePort);

		if(gamePort > std::numeric_limits<uint16_t>::max()) {
			Log::Error("[Network] [Server] Failed to bind game port socket");
			return;
		}

		ec = ConnectToMysql();

		RETURN_ON_ERROR(ec, "[Network] [Server] Failed to connect to MySQL: {0}");

		Config::Set<uint16_t>(L"network.server.gamePort:u16", static_cast<uint16_t>(gamePort));

		connection = std::make_shared<Connection>();
		connection->packetSequenceId = 1;
		connection->authorizedAt = SystemClock::LocalNow();
		connection->state = ConnectionState::ESTABLISHED;
		connection->liveEndpoint = UdpEndpoint{ addr, static_cast<uint16_t>(gamePort) };
		connection->socket = std::make_unique<UdpSocket>(std::move(gameSocket));
	}

	void ServerSession::Start() {
		GameSocketReadInit();

		Log::Info("[Network] [Server] Started on port: {0}", Config::Get<uint16_t>(L"network.server.gamePort:u16"));
	}

	void ServerSession::Stop()
	{
		dbContext.Stop();
	}
	
	Ref<Connection> ServerSession::MakeEmptyConnection() {
		return std::make_shared<Connection>();
	}
	
	void ServerSession::Update(int32_t subjectId, Protocol::ServerUpdate serverUpdate) {
		if(serverUpdate.has_time_sync()) {
			serverUpdate.mutable_time_sync()->set_server_resp_transmission(ConvertTimestampToUInt64(SystemClock::LocalNow()));
		}

		Ref<Connection> conn = connectionStorage.GetConnectionById(subjectId);

		if(conn->liveEndpoint.address().is_unspecified()) {
			Log::Debug("[Net] [Server] update failed because address is unspecified");
			return;
		}

		serverUpdate.set_id(conn->packetSequenceId++);
		
		auto packet = storage->GetBuffer();

		if(!serverUpdate.SerializeToArray(packet->GetData(), PACKET_STORAGE_SIZE)) {
			Log::Error("[Net] [Server] Failed to serialize server update");
		}

		packet->SetDataSize(serverUpdate.ByteSizeLong());
		packet->SetEndpoint(conn->liveEndpoint);
		
		connection->socket->async_send_to(packet->GetConstBuffer(), packet->GetEndpoint(), 
			[lifetime = shared_from_this(), packet](const ErrorCode & ec, size_t size) ->void {
			if(ec) {
				Log::Error("[Net] [Server] failed to send message to client: {0}", ec.message());
			} else {
				//Log::Debug("[---] WRITE {0}: BYTES {1} PORT {2}", static_cast<void *>(packet.get()), static_cast<uint64_t>(size), static_cast<uint64_t>(packet->GetEndpoint().port()));
			}
		});
		
	}

	void ServerSession::GameSocketReadInit()
	{
		auto packet = storage->GetBuffer();
		packet->SetDataSize(PACKET_STORAGE_SIZE);

		connection->socket->async_receive_from(packet->GetMutableBuffer(), packet->GetEndpoint(), [lifetime = shared_from_this(), this, packet](const ErrorCode & ec, size_t size) -> void {
			RETURN_AND_LOG_IF_ABORTED(ec, "[Net] [Server] [Game] Read operation aborted");

			if(ec) {
				Log::Error("[Net] [Server] [Game] Failed to read: {0}", ec.message());
			} else if(packet->GetEndpoint().address().is_unspecified()) {
				Log::Warn("[Net] [Server] [Game] Rejected packet because the source address is unspecified");
			} else {
				///Log::Debug("[---] READ {0}: BYTES {1} PORT {2}", static_cast<void *>(packet.get()), static_cast<uint64_t>(size), static_cast<uint64_t>(packet->GetEndpoint().port()));

				packet->SetTimestamp(SystemClock::LocalNow());
				packet->SetDataSize(size);
				OnGameRead(packet);
				GameSocketReadInit();
			}
		});
	}

	void ServerSession::OnGameRead(Ref<UdpPacket> packet) {
		Protocol::ClientUpdate message;

		if(!message.ParseFromArray(packet->GetData(), packet->GetDataSize())) {
			Log::Info("[Network] [Server] Failed to parse raw data from game socket, {0} byte(s)", packet->GetDataSize());
			return;
		}

		if(message.has_time_sync()) {
			message.mutable_time_sync()->set_server_req_reception(ConvertTimestampToUInt64(packet->GetTimestamp()));
		}
		
		Ref<Connection> conn = connectionStorage.GetConnectionByEndpoint(packet->GetEndpoint());

		// if connection was not found by endpoint ...
		if(conn == nullptr) {

			// ... then we require a nonce ...
			if(!message.nonce().empty()) {
				conn = connectionStorage.GetConnectionByNonce(message.nonce());

				// ... if we found a connection by nonce ...
				if(conn != nullptr) {

					// ... then we make sure we have the state required to be here ...
					if(conn->state == ConnectionState::CONNECTING ||
					   conn->state == ConnectionState::ESTABLISHED) {

						// ... if the state is acceptable, we check if its the first time this client is handled ...
						if(conn->liveEndpoint.address().is_unspecified()) {
							// ... if so, then we proceed it to clock sync ...
							conn->liveEndpoint = packet->GetEndpoint();
							conn->state = ConnectionState::SYNCHRONIZING;
							Log::Debug("[Net] [Server] Player first time here, synchronizing clocks");
						}

						// ... if its not the first time, than our clocks should be still adequate
						if(conn->liveEndpoint != packet->GetEndpoint()) {
							Log::Debug("[Net] [Server] Network change detected");
							conn->liveEndpoint = packet->GetEndpoint();
						}
					}
				} else {
					// we have a bad nonce here value here
					Log::Debug("[Net] [Server] Nonce value was specified but was not found");
					return;
				}
			} else {

				Log::Debug("[Net] [Server] player was not found by active connection, nor supplied a nonce");
				return; // ... if nonce was not supplied, then we drop the packet
			}
		} else {
			if(conn->state == ConnectionState::SYNCHRONIZING) {
				if(!message.has_time_sync()) {
					Log::Debug("[Net] [Server] Client should be synchronizing but did not sent a time sync package");
				}
			}
		}

		if(conn->peerPacketSequenceId < message.id()) {
			conn->peerPacketSequenceId = message.id();
			gameQueue.Received(std::move(message));
		} else {
			Log::Debug("[Net] [Server] Dropping player packet because it is older or duplicated");
		}
		
	}

	void ServerSession::OnMessageSent(const ErrorCode & ec, std::size_t size)
	{
		RETURN_AND_LOG_IF_ABORTED(ec, "[Network] [Server] Write operation aborted");

		if(ec) {
			Log::Error("[Network] [Server] Failed to send message: {0}", ec.message());
		} else {
			Log::Debug("[Network] [Server] Successfully sent {0} byte(s)", size);
		}
	}

	ServerSession::~ServerSession() {
		ErrorCode ec = db.CloseServer();

		RETURN_ON_ERROR(ec, "[Network] [Server] Failed to close server session, reason: {0}");

		Log::Info("[Network] [Server] Closed gracefully");
	}

	ErrorCode ServerSession::ConnectToMysql() {
		ErrorCode ec = db.Connect();

		if(ec) {
			return ec;
		}

		Log::Info("Connected to MySQL db");


		ec = db.RegisterServer(1, 8, 500, "::1", 50051, 8888);

		if(ec) {
			Log::Error("[Net] [Server] failed to register server");
			return ec;
		}
		
		dbContext.Start(1);
		
		return Errc::make_error_code(Errc::success);
	}

}
