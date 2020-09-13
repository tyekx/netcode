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
		gameStream{},
		storage{ std::make_shared<PacketStorage>() },
		db{},
		dbContext{}{

		udp_socket_t gameSocket{ ioc };

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

		gameStream = std::make_shared<UdpStream>(std::move(gameSocket));

		ec = ConnectToMysql();

		RETURN_ON_ERROR(ec, "[Network] [Server] Failed to connect to MySQL: {0}");

		Config::Set<uint16_t>(L"network.server.gamePort:u16", static_cast<uint16_t>(gamePort));
	}

	void ServerSession::Start() {
		GameSocketReadInit();

		Log::Info("[Network] [Server] Started on ports {0}", Config::Get<uint16_t>(L"network.server.gamePort:u16"));
	}

	void ServerSession::Stop()
	{
		dbContext.Stop();
	}
	void ServerSession::Update(int32_t subjectId, Protocol::ServerUpdate serverUpdate) {
		if(serverUpdate.has_time_sync()) {
			serverUpdate.mutable_time_sync()->set_server_resp_transmission(ConvertTimestampToUInt64(SystemClock::LocalNow()));
		}

		udp_endpoint_t tmpEndpoint{ boost::asio::ip::address::from_string("::1"), 8899 };

		auto buffer = storage->GetBuffer();

		if(!serverUpdate.SerializeToArray(buffer.get(), PACKET_STORAGE_SIZE)) {
			Log::Error("[Net] [Server] Failed to serialize server update");
		}
		
		gameStream->AsyncWrite(boost::asio::const_buffer{ buffer.get(), serverUpdate.ByteSizeLong() }, tmpEndpoint, 
			[buffer, lifetime = shared_from_this()](const ErrorCode & ec, size_t sz) -> void {
			if(ec) {
				Log::Error("[Net] [Server] failed to send message to client: {0}", ec.message());
			} else {
				Log::Info("[Net] [Server] sent {0} bytes", sz);
			}
		});
	}
	/*
	void ServerSession::Receive(std::vector<Protocol::Message> & game)
	{
		std::vector<UdpPacket> gameSockPackets;
		gameQueue.GetIncomingPackets(gameSockPackets);
		
		ParseGameMessages(game, std::move(gameSockPackets));
	}

	void ServerSession::SendAll() {
		std::vector<UdpPacket> controlPackets;
		std::vector<UdpPacket> gamePackets;
		gameQueue.GetOutgoingPackets(gamePackets);

		for(UdpPacket & p : gamePackets) {
			gameStream->AsyncWrite(p.GetConstBuffer(), p.GetEndpoint(),
				[pThis = shared_from_this(), pData = p.GetData(), this](const ErrorCode & ec, size_t size) -> void {
				OnMessageSent(ec, size);
			});
		}
	}

	void ServerSession::SendUpdate(Protocol::Message message) {
		Log::Warn("[Network] [Server] operation is not supported");
	}

	void ServerSession::SendUpdate(const udp_endpoint_t & endpoint, Protocol::Message message) {
		if(!message.has_server_update()) {
			Log::Warn("[Network] [Server] Can only send server updates on the game socket");
			return;
		}

		UdpPacket packet{ storage->GetBuffer(), PACKET_STORAGE_SIZE, endpoint };

		if(!message.SerializeToArray(packet.GetDataPointer(), packet.GetDataSize())) {
			Log::Error("[Network] [Server] Failed to serialize game message");
			return;
		}

		packet.SetDataSize(message.ByteSizeLong());

		gameQueue.Send(std::move(packet));
	}*/

	void ServerSession::GameSocketReadInit()
	{
		auto buffer = storage->GetBuffer();

		gameStream->AsyncRead(boost::asio::buffer(buffer.get(), PACKET_STORAGE_SIZE),
			[this, buffer, lifetime = shared_from_this()](const ErrorCode & ec, size_t size, udp_endpoint_t sender) -> void {

			RETURN_AND_LOG_IF_ABORTED(ec, "[Net] [Server] [Game] Read operation aborted");

			if(ec) {
				Log::Error("[Net] [Server] [Game] Failed to read: {0}", ec.message());
			} else if(sender.address().is_unspecified()) {
				Log::Warn("[Net] [Server] [Game] Rejected packet because the source address is unspecified");
			} else {
				Log::Info("[Net] [Server] Read {0} byte(s)", size);
				UdpPacket packet{ std::move(buffer), size, sender };
				packet.SetTimestamp(SystemClock::LocalNow());
				OnGameRead(std::move(packet));
				GameSocketReadInit();
				return;
			}
		});
	}

	void ServerSession::OnGameRead(UdpPacket packet) {
		// could parse now, async, multithreaded context
		gameQueue.Received(std::move(packet));
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

	void ServerSession::ParseGameMessages(std::vector<Protocol::ClientUpdate> & outVec, std::vector<UdpPacket> packets)
	{
		if(packets.empty()) {
			return;
		}

		for(UdpPacket & p : packets) {
			Protocol::ClientUpdate message;

			if(!message.ParseFromArray(p.GetDataPointer(), p.GetDataSize())) {
				Log::Info("[Network] [Server] Failed to parse raw data from game socket, {0} byte(s)", p.GetDataSize());
				continue;
			}

			//if(!message.has_player_state()) {
				//	Log::Info("[Network] [Server] Dropping packet because it is missing player state");
				//	continue;
				//}

			if(message.has_time_sync()) {
				auto *timeSync = message.mutable_time_sync();
				timeSync->set_server_req_reception(ConvertTimestampToUInt64(p.GetTimestamp()));
			}
			
			// @TODO: save id and endpoint
			//int32_t id = message.player_state().id();

			outVec.emplace_back(std::move(message));
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


		//Config::Get<uint16_t>(L"network.server.controlPort:u16");
		//Config::Get<uint16_t>(L"network.server.gamePort:u16");
		// @TODO: register server

		dbContext.Start(1);
		
		return Errc::make_error_code(Errc::success);
	}

}
