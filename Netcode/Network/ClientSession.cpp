#include "ClientSession.h"
#include "../Logger.h"

namespace Netcode::Network {
	void ClientSession::OnUpdateEndpointResolved(const boost::system::error_code & ec, udp_resolver_t::results_type results)
	{
		if(ec) {
			if(ec == boost::asio::error::operation_aborted) {
				Log::Info("[Network] [Client] Address resolution aborted");
			} else {
				Log::Error("[Network] [Client] Address resolution: {0}", ec.message());
			}
		} else if(results.empty()) {
			Log::Error("[Network] [Client] Address resolution: could not resolve address");
		} else {
			Log::Debug("[Network] [Client] Game endpoint resolution succeeded");

			for(auto result : results) {
				if(!result.endpoint().address().is_unspecified()) {
					updateEndpoint = result.endpoint();
					break;
				}
			}

			if(updateEndpoint.protocol() != controlEndpoint.protocol()) {
				Log::Error("[Network] [Client] game and control point uses different protocols, which is not supported by the client");
				SetError(boost::asio::error::make_error_code(boost::asio::error::address_family_not_supported));
				return;
			}

			udp_socket_t socket{ ioContext };
			socket.open(updateEndpoint.protocol());
			socket.bind(udp_endpoint_t{ updateEndpoint.protocol(), config.client.localPort });

			stream = std::make_unique<UdpStream>(std::move(socket));
			InitRead();
			InitTimer();
		}
	}

	void ClientSession::OnControlEndpointResolved(const boost::system::error_code & ec, udp_resolver_t::results_type results)
	{
		if(ec) {
			if(ec == boost::asio::error::operation_aborted) {
				Log::Info("[Network] [Client] Address resolution aborted");
			} else {
				Log::Error("[Network] [Client] Address resolution: {0}", ec.message());
			}
		} else if(results.empty()) {
			Log::Error("[Network] [Client] Address resolution: could not resolve address");
		} else {
			Log::Debug("[Network] [Client] Control endpoint resolution succeeded");

			for(auto result : results) {
				if(!result.endpoint().address().is_unspecified()) {
					controlEndpoint = result.endpoint();
					break;
				}
			}

			resolver.async_resolve(config.client.serverHost, updatePort, udp_resolver_t::address_configured,
				boost::bind(&ClientSession::OnUpdateEndpointResolved, this, boost::asio::placeholders::error, boost::asio::placeholders::results));
		}
	}

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
		timer.expires_from_now(boost::posix_time::milliseconds(config.client.tickIntervalMs));
		timer.async_wait(boost::bind(&ClientSession::OnTimerExpired, this, boost::asio::placeholders::error));
	}

	void ClientSession::InitResolution()
	{
		controlPort = std::to_string(config.client.serverControlPort);
		updatePort = std::to_string(config.client.serverGamePort);

		resolver.async_resolve(config.client.serverHost, controlPort, udp_resolver_t::address_configured,
			boost::bind(&ClientSession::OnControlEndpointResolved, this, boost::asio::placeholders::error, boost::asio::placeholders::results));
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
			[this, buffer](const boost::system::error_code & ec, std::size_t size, boost::asio::ip::udp::endpoint endpoint) -> void {
			if(ec == boost::asio::error::operation_aborted) {
				Log::Info("[Network] [Client] Read operation aborted");
				return;
			}

			if(ec) {
				Log::Error("[Network] [Client] Failed to read: {0}", ec.message());
			} else if(endpoint != updateEndpoint && endpoint != controlEndpoint) {
				Log::Warn("[Network] [Client] Rejected package because the source is not the server");
			} else {
				Log::Debug("[Network] [Client] Read {0} byte(s)", size);
				OnRead(size, endpoint, std::move(buffer));
				InitRead();
			}
		});
	}

	ClientSession::ClientSession(boost::asio::io_context & ioc, Network::Config config) :
		ioContext{ ioc }, config{ config }, queue{ }, gameQueue{ }, controlQueue{ }, storage{ }, controlStorage{ }, timer{ ioc }, protocolTimer{ ioc },
		resolver{ boost::asio::make_strand(ioc) }, stream{ nullptr }, controlPort{ }, updatePort{ }, controlEndpoint{ },
		updateEndpoint{ }, clientAck{ 0 }, serverAck{ 0 }, lastError{ }
	{
		InitResolution();
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
			bool parseSuccess = message.ParseFromArray(p.mBuffer.data(), p.mBuffer.size());

			if(!parseSuccess) {
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
					Log::Debug("[Network] [Client] Ack received: {0}", ack);
					controlStorage.Acknowledge(ack);
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

				controlQueue.Received(std::move(message));
			} else if(message.has_server_update()) {
				gameQueue.Received(std::move(message));
			}
		}

		controlStorage.CheckTimeouts(config.protocol.gracePeriodMs);

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

	std::future<ErrorCode> ClientSession::SendControlMessage(Netcode::Protocol::Message message)
	{
		std::promise<ErrorCode> errorPromise;
		std::future<ErrorCode> errorFuture = errorPromise.get_future();
		if(!message.has_control()) {
			Log::Warn("[Network] [Client] Can not send a control message without a control body");
			errorPromise.set_value(Errc::make_error_code(Errc::operation_not_permitted));
			return errorFuture;
		}

		using Ctrl = Protocol::Control::Message;

		if(message.control().Body_case() != Ctrl::BodyCase::kRequest) {
			Log::Warn("[Network] [Client] Can only send requests");
			errorPromise.set_value(Errc::make_error_code(Errc::operation_not_permitted));
			return errorFuture;
		}

		int32_t controlId = clientAck++;

		message.set_id(controlId);

		auto buffer = storage.GetBuffer();

		bool v = message.SerializeToArray(buffer.get(), PACKET_STORAGE_SIZE);

		if(!v) {
			Log::Error("[Network] [Client] Failed to serialize Control message");
			errorPromise.set_value(Errc::make_error_code(Errc::bad_message));
			return errorFuture;
		}

		UdpPacket packet{ std::move(buffer) };
		packet.endpoint = controlEndpoint;
		packet.mBuffer = boost::asio::buffer(packet.data.get(), message.ByteSizeLong());

		return controlStorage.Push(controlId, std::move(packet));
	}
	
	void ClientSession::SendAll() {
		controlStorage.IfExpired([this](UdpPacket & packet) -> void {
			std::string addr = controlEndpoint.address().to_string();
			addr += std::string{ ":" } + std::to_string(controlEndpoint.port());

			Log::Debug("[Network] [Client] SendAll: sending expired control packet to {0}", addr);

			stream->AsyncWrite(packet.mBuffer, controlEndpoint, [](const ErrorCode & ec, std::size_t size) -> void {
				if(ec == boost::asio::error::operation_aborted) {
					Log::Info("[Network] [Client] Write operation aborted");
					return;
				}

				if(ec) {
					Log::Error("[Network] [Client] Failed to send message {0}", ec.message());
				} else {
					Log::Debug("[Network] [Client] Successfully wrote {0} byte(s) to the control socket", size);
				}
			});

		}, config.protocol.resendTimeoutMs);
	}
}
