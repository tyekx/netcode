#pragma once 

#include "GameSession.h"
#include "NetworkCommon.h"
#include <boost/asio.hpp>

namespace Netcode::Network {

	class ClientSession : public GameSession {
		boost::asio::io_context & ioContext;
		Network::Config config;
		MessageQueue<UdpPacket> queue;
		MessageQueue<Netcode::Protocol::Message> gameQueue;
		MessageQueue<Netcode::Protocol::Message> controlQueue;
		PacketStorage storage;
		ControlPacketStorage controlStorage;
		boost::asio::deadline_timer timer;
		boost::asio::deadline_timer protocolTimer;
		udp_resolver_t resolver;
		std::unique_ptr<UdpStream> stream;
		std::string controlPort;
		std::string updatePort;
		udp_endpoint_t controlEndpoint;
		udp_endpoint_t updateEndpoint;
		int32_t clientAck;
		int32_t serverAck;
		boost::system::error_code lastError;

		void SetError(const boost::system::error_code & ec);

		void Tick();

		void OnUpdateEndpointResolved(const boost::system::error_code & ec, udp_resolver_t::results_type results);
		void OnControlEndpointResolved(const boost::system::error_code & ec, udp_resolver_t::results_type results);

		void OnTimerExpired(const boost::system::error_code & ec);

		void InitTimer();
		void InitResolution();
		void InitRead();
		void OnRead(std::size_t transferredBytes, udp_endpoint_t endpoint, PacketStorage::StorageType buffer);
	public:
		ClientSession(boost::asio::io_context & ioc, Network::Config config);

		virtual bool IsRunning() const override;
		virtual std::string GetLastError() const override;
		virtual void Receive(std::vector<Protocol::Message> & control, std::vector<Protocol::Message> & game) override;
		virtual void SendUpdate(const udp_endpoint_t & endpoint, Protocol::Message message) override;
		virtual void SendUpdate(Protocol::Message message) override;
		virtual std::future<ErrorCode> SendControlMessage(Netcode::Protocol::Message message) override;
	};

}
