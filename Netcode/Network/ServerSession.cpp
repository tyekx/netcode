#include <mysqlx/xdevapi.h>
#include <boost/asio.hpp>

#include "../Logger.h"
#include "ServerSession.h"

#define RETURN_IF(condition) if(condition) return

#define RETURN_ON_ERROR(ec, msg)	\
if(ec) {	\
	Log::Error(msg, ec.message()); \
	lastError = ec;	\
	return;	\
}

#define RETURN_VALUE_ON_ERROR(ec, msg, rv)	\
if(ec) {	\
	Log::Error(msg, ec.message()); \
	lastError = ec;	\
	return rv;	\
}

namespace Netcode::Network {

	ServerSession::ServerSession(boost::asio::io_context & ioc, Network::Config cfg) :
		ioContext{ ioc },
		timer{ ioc },
		config{ cfg },
		controlQueue{},
		gameQueue{},
		controlStream{},
		gameStream{},
		storage{},
		db{},
		lastError{} {

		udp_socket_t controlSocket{ ioc };
		udp_socket_t gameSocket{ ioc };

		const uint32_t controlPort = ServerSession::BindToPort(controlSocket, config.server.controlPort);

		RETURN_IF(lastError);

		const uint32_t gamePort = ServerSession::BindToPort(gameSocket, config.server.gamePort);

		RETURN_IF(lastError);

		if(controlPort > std::numeric_limits<uint16_t>::max() || gamePort > std::numeric_limits<uint16_t>::max()) {
			Log::Error("[Network] [Server] Failed to bind control, or game port socket");
			lastError = boost::asio::error::make_error_code(boost::asio::error::bad_descriptor);
			return;
		}

		config.server.controlPort = controlPort;
		config.server.gamePort = gamePort;

		controlStream = std::make_unique<UdpStream>(std::move(controlSocket));
		gameStream = std::make_unique<UdpStream>(std::move(gameSocket));

		ConnectToMysql();

		RETURN_IF(lastError);

		ControlSocketReadInit();

		GameSocketReadInit();

		Log::Info("[Network] [Server] Started on ports {0}, {1}",
			config.server.controlPort, config.server.gamePort);
	}

	void ServerSession::Receive(std::vector<Protocol::Message> & control, std::vector<Protocol::Message> & game)
	{
		std::vector<UdpPacket> controlSockPackets;
		controlQueue.GetIncomingPackets(controlSockPackets);

		std::vector<UdpPacket> gameSockPackets;
		gameQueue.GetIncomingPackets(gameSockPackets);
		
		ParseControlMessages(control, controlSockPackets);
		ParseGameMessages(game, gameSockPackets);

		
	}

	uint32_t ServerSession::BindToPort(udp_socket_t & socket, uint16_t portHint)
	{
		// below 1024: well known ports
		// above 49151: dynamically registered ports
		if(portHint < 1024 || portHint > 49151) {
			portHint = (49151 - 1024) / 2;
		}

		ErrorCode ec;
		const uint32_t range = 49151 - 1024;
		const int32_t start = static_cast<int32_t>(portHint);
		int32_t sign = 1;
		
		boost::asio::ip::address serverAddr = boost::asio::ip::address::from_string(config.server.selfAddress, ec);

		RETURN_VALUE_ON_ERROR(ec, "[Network] The supplied ip address (srv.address) is invalid: {0}", std::numeric_limits<uint32_t>::max());

		udp_endpoint_t endpoint{ serverAddr, 0 };

		socket.open(endpoint.protocol(), ec);

		RETURN_VALUE_ON_ERROR(ec, "[Network] Failed to open socket, reason: {0}", std::numeric_limits<uint32_t>::max());

		for(int32_t i = 0; i < range; ++i, sign = -sign) {
			uint32_t portToTest = static_cast<uint32_t>(start + sign * i / 2);

			if(portToTest < 1024 || portToTest > 49151) {
				continue;
			}

			endpoint.port(portToTest);

			socket.bind(endpoint, ec);
			
			if(ec == boost::asio::error::bad_descriptor) { // expected error
				continue;
			} else if(ec) { // unexpected error 
				Log::Error("[Network] Failed to bind socket: {0}", ec.message());
				lastError = ec;
				return std::numeric_limits<uint32_t>::max();
			} else { // success
				return portToTest;
			}
		}

		return std::numeric_limits<uint32_t>::max();
	}

	void ServerSession::ControlSocketReadInit()
	{
		auto buffer = storage.GetBuffer();
		Log::Debug("[Network] [Server] ControlSocketReadInit()");

		if(buffer == nullptr) {
			Log::Error("[Network] [Server] buffer is nullptr");
		}

		controlStream->AsyncRead(boost::asio::buffer(buffer.get(), PACKET_STORAGE_SIZE),
			[this, buffer](const ErrorCode & ec, std::size_t size, udp_endpoint_t sender) -> void {
			if(ec == boost::asio::error::operation_aborted) {
				Log::Info("[Network] [Server] [Control] Read operation aborted");
				return;
			}

			if(ec) {
				Log::Error("[Network] [Server] [Control] Failed to read: {0}", ec.message());
			} else if(sender.address().is_unspecified()) {
				Log::Warn("[Network] [Server] [Control] Rejected packet because the source address is unspecified");
			} else {
				Log::Debug("[Network] [Server] [Control] Received {0} byte(s)", size);
				UdpPacket packet;
				packet.data = std::move(buffer);
				packet.endpoint = std::move(sender);
				packet.mBuffer = boost::asio::buffer(buffer.get(), size);
				OnControlRead(std::move(packet));
				ControlSocketReadInit();
				return;
			}

			if(buffer != nullptr) {
				// on error: return to storage
				storage.ReturnBuffer(std::move(buffer));
			}
		});
	}

	void ServerSession::SendAll() {
		std::vector<UdpPacket> controlPackets;
		std::vector<UdpPacket> gamePackets;
		controlQueue.GetOutgoingPackets(controlPackets);
		gameQueue.GetOutgoingPackets(gamePackets);

		for(UdpPacket & p : controlPackets) {
			controlStream->AsyncWrite(p.mBuffer, p.endpoint, boost::bind(
				&ServerSession::OnMessageSent,
				this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				p.data
			));
		}

		for(UdpPacket & p : gamePackets) {
			gameStream->AsyncWrite(p.mBuffer, p.endpoint, boost::bind(
				&ServerSession::OnMessageSent,
				this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				p.data
			));
		}
	}

	void ServerSession::GameSocketReadInit()
	{
		auto buffer = storage.GetBuffer();

		controlStream->AsyncRead(boost::asio::buffer(buffer.get(), PACKET_STORAGE_SIZE),
			[this, buffer](const ErrorCode & ec, std::size_t size, udp_endpoint_t sender) -> void {
			if(ec == boost::asio::error::operation_aborted) {
				Log::Info("[Network] [Server] [Game] Read operation aborted");
				return;
			}

			if(ec) {
				Log::Error("[Network] [Server] [Game] Failed to read: {0}", ec.message());
			} else if(sender.address().is_unspecified()) {
				Log::Warn("[Network] [Server] [Game] Rejected packet because the source address is unspecified");
			} else {
				UdpPacket packet;
				packet.data = std::move(buffer);
				packet.endpoint = std::move(sender);
				packet.mBuffer = boost::asio::buffer(buffer.get(), size);
				OnGameRead(std::move(packet));
				GameSocketReadInit();
				return;
			}

			if(buffer != nullptr) {
				// on error: return to storage
				storage.ReturnBuffer(std::move(buffer));
			}
		});
	}

	void ServerSession::OnGameRead(UdpPacket packet) {
		gameQueue.Received(std::move(packet));
	}

	void ServerSession::OnControlRead(UdpPacket packet) {
		controlQueue.Received(std::move(packet));
	}

	void ServerSession::OnMessageSent(const ErrorCode & ec, std::size_t size, PacketStorage::StorageType buffer)
	{
		if(ec == boost::asio::error::operation_aborted) {
			Log::Info("[Network] [Server] Write operation aborted");
			return;
		}
 
		if(ec) {
			Log::Error("[Network] [Server] Failed to send message: {0}", ec.message());
		} else {
			Log::Debug("[Network] [Server] Successfully sent {0} byte(s)", size);
		}
		storage.ReturnBuffer(buffer);
	}

	void ServerSession::ParseControlMessages(std::vector<Protocol::Message> & outVec, std::vector<UdpPacket> packets) {
		if(packets.empty()) {
			return;
		}

		std::vector<PacketStorage::StorageType> buffersToReturn;
		buffersToReturn.reserve(packets.size());

		for(UdpPacket & p : packets) {
			Protocol::Message message;

			buffersToReturn.emplace_back(p.data);

			if(!message.ParseFromArray(p.mBuffer.data(), p.mBuffer.size())) {
				Log::Info("[Network] [Server] Failed to parse raw data from control socket, {0} byte(s)", p.mBuffer.size());
				continue;
			}

			if(!message.has_control()) {
				Log::Info("[Network] [Server] Dropping packet because it is not a control message on the control socket");
				continue;
			}

			using Netcode::Protocol::Control::Message;

			const Message & ctrl = message.control();

			if(ctrl.Body_case() == Message::BodyCase::kAcknowledge) {
				int32_t acknowledged = message.control().acknowledge();
				controlStorage.Acknowledge(p.endpoint, acknowledged);
				continue;
			}

			if(ctrl.Body_case() == Message::BodyCase::kCommand ||
			   ctrl.Body_case() == Message::BodyCase::kResponse) {
				Log::Info("[Network] [Server] dropping control packet because the client can not send command or response typed messages");
				continue;
			}

			SendAck(p.endpoint, message.id());

			message.set_address(p.endpoint.address().to_string());
			message.set_port(p.endpoint.port());

			outVec.emplace_back(std::move(message));
		}

		storage.ReturnBuffer(std::move(buffersToReturn));
	}

	void ServerSession::ParseGameMessages(std::vector<Protocol::Message> & outVec, std::vector<UdpPacket> packets)
	{
		if(packets.empty()) {
			return;
		}

		std::vector<PacketStorage::StorageType> buffersToReturn;
		buffersToReturn.reserve(packets.size());

		for(UdpPacket & p : packets) {
			Protocol::Message message;

			buffersToReturn.emplace_back(p.data);

			if(!message.ParseFromArray(p.mBuffer.data(), p.mBuffer.size())) {
				Log::Info("[Network] [Server] Failed to parse raw data from game socket, {0} byte(s)", p.mBuffer.size());
				continue;
			}

			if(!message.has_client_update()) {
				Log::Info("[Network] [Server] Dropping packet because it is not a client update on the game port");
				continue;
			}

			message.set_address(p.endpoint.address().to_string());
			message.set_port(p.endpoint.port());

			outVec.emplace_back(std::move(message));
		}

		storage.ReturnBuffer(std::move(buffersToReturn));
	}

	void ServerSession::SendAck(udp_endpoint_t endpoint, int32_t ack)
	{
		auto buffer = storage.GetBuffer();
		UdpPacket packet{ std::move(buffer) };
		packet.endpoint = endpoint;

		Protocol::Message message;
		message.mutable_control()->set_acknowledge(ack);

		if(!message.SerializeToArray(packet.mBuffer.data(), packet.mBuffer.size())) {
			Log::Error("[Network] [Server] Failed to serialize ack message");
			return;
		}

		packet.mBuffer = boost::asio::buffer(packet.data.get(), message.ByteSizeLong());

		controlQueue.Send(std::move(packet));
	}

	ServerSession::~ServerSession() {
		ErrorCode ec = db.CloseServer();

		RETURN_ON_ERROR(ec, "[Network] [Server] Failed to close server session, reason: {0}");

		Log::Info("[Network] [Server] Closed gracefully");
	}

	void ServerSession::ConnectToMysql() {
		ErrorCode ec = db.Connect(config.database);

		RETURN_ON_ERROR(ec, "[Network] [Server] Failed to connect to mysql, reason: {0}");

		ec = db.RegisterServer(config.server);

		RETURN_ON_ERROR(ec, "[Network] [Server] Failed to register server, reason: {0}");
	}

}
