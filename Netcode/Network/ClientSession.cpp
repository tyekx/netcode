#include "ClientSession.h"
#include "../Logger.h"
#include "Macros.h"

#include "../Config.h"

namespace Netcode::Network {

	void ClientSession::OnTimerExpired(const boost::system::error_code & ec)
	{
		if(ec == boost::asio::error::operation_aborted) {
			Log::Info("[Network] [Client] Tick operation aborted");
			return;
		}

		if(ec) {
			Log::Error("[Network] [Client] Timer: {0}", ec.message());
		} else {
			Tick();
			InitTimer();
		}
	}

	void ClientSession::InitTimer()
	{
		timer.expires_from_now(boost::posix_time::milliseconds(Config::Get<uint32_t>("network.client.tickIntervalMs:u32")));
		timer.async_wait(boost::bind(&ClientSession::OnTimerExpired, GetStrongRef(), boost::asio::placeholders::error));
	}


	void ClientSession::OnRead(std::size_t transferredBytes, boost::asio::ip::udp::endpoint endpoint, PacketStorage::StorageType buffer) {
		UdpPacket packet;
		packet.endpoint = endpoint;
		packet.data = std::move(buffer);
		packet.mBuffer = boost::asio::buffer(packet.data.get(), transferredBytes);

		queue.Received(std::move(packet));
	}

	void ClientSession::InitRead() {
		auto buffer = storage.GetBuffer();

		Log::Debug("[Network] [Client] Initiating read");

		stream->AsyncRead(boost::asio::buffer(buffer.get(), PACKET_STORAGE_SIZE),
			[pThis = GetStrongRef(), buffer](const boost::system::error_code & ec, std::size_t size, boost::asio::ip::udp::endpoint endpoint) -> void {

			RETURN_AND_LOG_IF_ABORTED(ec, "[Network] [Client] Read operation aborted");

			if(ec) {
				Log::Error("[Network] [Client] Failed to read: {0}", ec.message());
			} else if(endpoint != pThis->updateEndpoint && endpoint != pThis->controlEndpoint) {
				Log::Warn("[Network] [Client] Rejected package because the source is not the server");
			} else {
				Log::Debug("[Network] [Client] Read {0} byte(s)", size);
				pThis->OnRead(size, endpoint, std::move(buffer));
				pThis->InitRead();
			}
		});
	}

	ClientSession::ClientSession(boost::asio::io_context & ioc) :
		ioContext{ ioc }, queue{ }, gameQueue{ }, controlQueue{ }, storage{ }, controlStorage{ }, timer{ ioc }, protocolTimer{ ioc },
		resolver{ boost::asio::make_strand(ioc) }, stream{ nullptr }, controlEndpoint{ },
		updateEndpoint{ }, clientAck{ 0 }, serverAck{ 0 }, lastError{ }
	{
		ErrorCode ec;

		std::string controlPortString = std::to_string(Config::Get<uint16_t>("network.client.controlPort:u16"));
		std::string gamePortString = std::to_string(Config::Get<uint16_t>("network.client.gamePort:u16"));

		auto controlResults = resolver.resolve(Config::Get<std::string>("network.client.hostname:string"), controlPortString, udp_resolver_t::address_configured, ec);

		RETURN_ON_ERROR(ec, "[Network] [Client] address resolution failed");

		RETURN_IF_AND_LOG_ERROR(controlResults.empty(), "[Network] [Client] address resolution failed");

		controlEndpoint = (*controlResults.begin());

		auto gameResults = resolver.resolve(Config::Get<std::string>("network.client.hostname:string"), gamePortString, udp_resolver_t::address_configured, ec);

		RETURN_ON_ERROR(ec, "[Network] [Client] address resolution failed: {0}");

		RETURN_IF_AND_LOG_ERROR(gameResults.empty(), "[Network] [Client] address resolution failed");

		updateEndpoint = (*gameResults.begin());

		RETURN_IF_AND_LOG_ERROR(updateEndpoint.protocol() != controlEndpoint.protocol(), "[Network] [Client] Protocol mismatch");

		udp_socket_t socket{ ioContext };
		socket.open(updateEndpoint.protocol(), ec);
		RETURN_ON_ERROR(ec, "[Network] [Client] failed to bind open: {0}");

		socket.bind(udp_endpoint_t{ updateEndpoint.protocol(), Config::Get<uint16_t>("network.client.localPort:u16") }, ec);
		RETURN_ON_ERROR(ec, "[Network] [Client] failed to bind socket: {0}");

		stream = std::make_shared<UdpStream>(std::move(socket));
	}

	void ClientSession::SendAck(int32_t ack)
	{
		auto buffer = storage.GetBuffer();
		UdpPacket packet{ std::move(buffer) };
		packet.endpoint = controlEndpoint;

		Protocol::Message message;
		message.mutable_control()->set_acknowledge(ack);

		if(!message.SerializeToArray(packet.mBuffer.data(), packet.mBuffer.size())) {
			Log::Error("[Network] [Server] Failed to serialize ack message");
			return;
		}

		packet.mBuffer = boost::asio::buffer(packet.data.get(), message.ByteSizeLong());

		queue.Send(std::move(packet));
	}

	void ClientSession::Start()
	{
		RETURN_IF(lastError);

		InitRead();

		InitTimer();

		Log::Info("[Network] [Client] Started on port {0}", Config::Get<uint16_t>("network.client.localPort:u16"));
	}

	bool ClientSession::CheckVersion(const Netcode::Protocol::Version & version)
	{
		return true;
	}

	bool ClientSession::IsRunning() const
	{
		return !((bool)lastError);
	}

	std::string ClientSession::GetLastError() const
	{
		return lastError.message();
	}

	void ClientSession::SetError(const boost::system::error_code & ec)
	{
		timer.cancel();
		protocolTimer.cancel();

		lastError = ec;
	}

	void ClientSession::Tick()
	{
		std::vector<UdpPacket> packets;
		queue.GetIncomingPackets(packets);

		for(const UdpPacket & p : packets) {
			Netcode::Protocol::Message message;

			if(!message.ParseFromArray(p.mBuffer.data(), p.mBuffer.size())) {
				Log::Debug("[Network] [Client] Failed to parse message from stream");
				continue;
			}

			if(message.has_client_update()) {
				Log::Warn("[Network] [Client] Message has client update body, which is unexpected from the server, dropping packet");
				continue;
			}

			if(message.has_control()) {
				using Netcode::Protocol::Control::Message;

				const auto & ctrl = message.control();
				auto bodyCase = ctrl.Body_case();

				if(bodyCase == Message::BodyCase::kAcknowledge) {
					int32_t ack = ctrl.acknowledge();
					ErrorCode ec = controlStorage.Acknowledge(p.endpoint, ack);

					if(ec) {
						Log::Error("[Network] [Client] A valid ACK message was received but the operation failed: {0}", ec.message());
					} else {
						Log::Debug("[Network] [Client] Received ACK: {0}", ack);
					}

					continue;
				}

				if(bodyCase == Message::BodyCase::kRequest) {
					Log::Warn("[Network] [Client] Request can not be received from the server, dropping packet");
					continue;
				}

				if(bodyCase == Message::BodyCase::BODY_NOT_SET) {
					Log::Warn("[Network] [Client] Message body is not set, dropping packet");
					continue;
				}

				SendAck(message.id());

				controlQueue.Received(std::move(message));
			} else if(message.has_server_update()) {
				gameQueue.Received(std::move(message));
			}
		}

		controlStorage.CheckTimeouts(Config::Get<uint32_t>("network.protocol.gracePeriodMs:u32"));

		SendAll();
	}

	void ClientSession::Receive(std::vector<Protocol::Message> & control, std::vector<Protocol::Message> & game)
	{
		controlQueue.GetIncomingPackets(control);
		gameQueue.GetIncomingPackets(game);
	}

	void ClientSession::SendUpdate(const udp_endpoint_t & endpoint, Protocol::Message message) {
		Log::Warn("[Network] [Client] operation is not supported");
	}

	void ClientSession::SendUpdate(Protocol::Message message) {

	}

	void ClientSession::SendControlMessage(Netcode::Protocol::Message message, std::function<void(ErrorCode)> completionHandler)
	{
		if(!message.has_control()) {
			Log::Warn("[Network] [Client] Can not send a control message without a control body");
			completionHandler(boost::asio::error::make_error_code(boost::asio::error::invalid_argument));
			return;
		}

		using Ctrl = Protocol::Control::Message;

		if(message.control().Body_case() != Ctrl::BodyCase::kRequest) {
			Log::Warn("[Network] [Client] Can only send requests");
			completionHandler(boost::asio::error::make_error_code(boost::asio::error::invalid_argument));
			return;
		}

		int32_t controlId = ++clientAck;

		message.set_id(controlId);

		auto buffer = storage.GetBuffer();

		if(!message.SerializeToArray(buffer.get(), PACKET_STORAGE_SIZE)) {
			Log::Error("[Network] [Client] Failed to serialize Control message");
			completionHandler(boost::asio::error::make_error_code(boost::asio::error::message_size));
			return;
		}

		UdpPacket packet{ std::move(buffer) };
		packet.endpoint = controlEndpoint;
		packet.mBuffer = boost::asio::buffer(packet.data.get(), message.ByteSizeLong());

		controlStorage.Push(controlId, std::move(packet), std::move(completionHandler));
	}
	
	void ClientSession::OnMessageSent(const ErrorCode & ec, std::size_t size, PacketStorage::StorageType buffer)
	{
		RETURN_AND_LOG_IF_ABORTED(ec, "[Network] [Server] Write operation aborted");

		if(ec) {
			Log::Error("[Network] [Server] Failed to send message: {0}", ec.message());
		} else {
			Log::Debug("[Network] [Server] Successfully sent {0} byte(s)", size);
		}

		storage.ReturnBuffer(buffer);
	}

	void ClientSession::SendAll() {
		controlStorage.ForeachExpired([this](UdpPacket & packet) -> void {
			std::string addr = controlEndpoint.address().to_string();
			addr += std::string{ ":" } + std::to_string(controlEndpoint.port());

			Log::Debug("[Network] [Client] SendAll: sending expired control packet to {0}", addr);

			stream->AsyncWrite(packet.mBuffer, controlEndpoint, [pThis = GetStrongRef()](const ErrorCode & ec, std::size_t size) -> void {
				RETURN_AND_LOG_IF_ABORTED(ec, "[Network] [Client] Write operation aborted");

				if(ec) {
					Log::Error("[Network] [Client] Failed to send message {0}", ec.message());
				} else {
					Log::Debug("[Network] [Client] Successfully wrote {0} byte(s) to the control socket", size);
				}
			});

		}, Config::Get<uint32_t>("network.protocol.resendTimeoutMs:u32"));

		std::vector<UdpPacket> outgoing;

		queue.GetOutgoingPackets(outgoing);

		for(auto & p : outgoing) {
			stream->AsyncWrite(p.mBuffer, p.endpoint, boost::bind(
				&ClientSession::OnMessageSent,
				GetStrongRef(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred,
				p.data
			));
		}
	}
}
