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


	void ClientSession::OnRead(size_t transferredBytes, boost::asio::ip::udp::endpoint endpoint, Ref<uint8_t[]> buffer) {
		UdpPacket packet{ std::move(buffer), transferredBytes, std::move(endpoint) };
		queue.Received(std::move(packet));
	}

	void ClientSession::InitRead() {
		auto buffer = storage->GetBuffer();

		Log::Debug("[Net] [Client] Initiating read");

		stream->AsyncRead(boost::asio::buffer(buffer.get(), PACKET_STORAGE_SIZE),
			[this, buffer, lifetime = shared_from_this()](const boost::system::error_code & ec, size_t size, boost::asio::ip::udp::endpoint endpoint) -> void {

			RETURN_AND_LOG_IF_ABORTED(ec, "[Net] [Client] Read operation aborted");

			if(ec) {
				Log::Error("[Net] [Client] Failed to read: {0}", ec.message());
			} else if(endpoint != updateEndpoint) {
				Log::Warn("[Net] [Client] Rejected package because the source is not the server");
			} else {
				Log::Debug("[Net] [Client] Read {0} byte(s)", size);
				OnRead(size, endpoint, std::move(buffer));
				InitRead();
			}
		});
	}

	ClientSession::ClientSession(boost::asio::io_context & ioc) :
		ioContext{ ioc }, queue{ }, gameQueue{ }, storage{ std::make_shared<PacketStorage>() }, timer{ ioc }, protocolTimer{ ioc },
		resolver{ boost::asio::make_strand(ioc) }, stream{ nullptr },
		updateEndpoint{ }
	{
		ErrorCode ec;

		std::string gamePortString = std::to_string(Config::Get<uint16_t>(L"network.client.gamePort:u16"));

		std::string hostname = Utility::ToNarrowString(Config::Get<std::wstring>(L"network.client.hostname:string"));
		
		auto gameResults = resolver.resolve(hostname, gamePortString, udp_resolver_t::address_configured, ec);

		RETURN_ON_ERROR(ec, "[Net] [Client] address resolution failed: {0}");

		RETURN_IF_AND_LOG_ERROR(gameResults.empty(), "[Net] [Client] address resolution failed");

		updateEndpoint = (*gameResults.begin());

		udp_socket_t socket{ ioContext };
		socket.open(updateEndpoint.protocol(), ec);
		RETURN_ON_ERROR(ec, "[Net] [Client] failed to bind open: {0}");

		socket.bind(udp_endpoint_t{ updateEndpoint.protocol(), Config::Get<uint16_t>(L"network.client.localPort:u16") }, ec);
		RETURN_ON_ERROR(ec, "[Net] [Client] failed to bind socket: {0}");

		stream = std::make_shared<UdpStream>(std::move(socket));
	}

	void ClientSession::Start()
	{
		InitRead();

		InitTimer();

		Log::Info("[Net] [Client] Started on port {0}", Config::Get<uint16_t>(L"network.client.localPort:u16"));
	}

	void ClientSession::Update(Protocol::ClientUpdate message) {
		Ref<uint8_t[]> buffer = storage->GetBuffer();

		if(!message.SerializeToArray(buffer.get(), PACKET_STORAGE_SIZE)) {
			Log::Error("[Net] [Client] Failed to serialize to array");
			return;
		}

		stream->AsyncWrite(boost::asio::const_buffer{ buffer.get(), message.ByteSizeLong() }, updateEndpoint, 
			[buffer, lifetime = shared_from_this()](const ErrorCode & ec, size_t sz) -> void {
			if(ec) {
				Log::Error("[Net] [Client] Failed to send message: {0}", ec.message());
			} else {
				Log::Info("[Net] [Client] sent {0} bytes", static_cast<int32_t>(sz));
			}
		});
	}

	void ClientSession::SetError(const boost::system::error_code & ec)
	{
		timer.cancel();
		protocolTimer.cancel();
	}

	void ClientSession::Tick()
	{
		std::vector<UdpPacket> packets;
		queue.GetIncomingPackets(packets);

		for(const UdpPacket & p : packets) {
			Protocol::ServerUpdate message;

			if(!message.ParseFromArray(p.GetDataPointer(), p.GetDataSize())) {
				Log::Debug("[Net] [Client] Failed to parse message from stream");
				continue;
			}
			
			
			gameQueue.Received(std::move(message));
		}

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
	/*
	void ClientSession::Receive(std::vector<Protocol::Message> & game)
	{
		gameQueue.GetIncomingPackets(game);
	}

	void ClientSession::SendUpdate(Protocol::Message message) {

	}

	void ClientSession::SendAll() {
		std::vector<UdpPacket> outgoing;

		queue.GetOutgoingPackets(outgoing);

		for(UdpPacket & p : outgoing) {
			stream->AsyncWrite(boost::asio::mutable_buffer{ p.GetDataPointer(), p.GetDataSize() }, p.GetEndpoint(),
			[this, buffer = p.GetData(), lifetime = shared_from_this()](const ErrorCode &ec, size_t sz) ->void {
				OnMessageSent(ec, sz);
			});
		}
	}
	*/
}
