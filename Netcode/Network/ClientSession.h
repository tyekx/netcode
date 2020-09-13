#pragma once 

#include "../ModulesConfig.h"
#include "GameSession.h"
#include "NetworkCommon.h"
#include <boost/asio.hpp>

namespace Netcode::Network {

	class ClientSession : public ClientSessionBase {
		boost::asio::io_context & ioContext;
		MessageQueue<UdpPacket> queue;
		MessageQueue<Netcode::Protocol::ServerUpdate> gameQueue;
		Ref<PacketStorage> storage;
		boost::asio::deadline_timer timer;
		boost::asio::deadline_timer protocolTimer;
		udp_resolver_t resolver;
		Ref<UdpStream> stream;
		udp_endpoint_t updateEndpoint;
		
		void SetError(const boost::system::error_code & ec);

		void Tick();

		void OnTimerExpired(const boost::system::error_code & ec);

		void InitTimer();
		void InitRead();
		void OnRead(size_t transferredBytes, udp_endpoint_t endpoint, Ref<uint8_t[]> buffer);
		void OnMessageSent(const ErrorCode & ec, size_t size);

	public:
		ClientSession(boost::asio::io_context & ioc);
		virtual ~ClientSession() = default;
		virtual void Start() override;
		
		virtual void Stop() override {

		}

		virtual void SwapBuffers(std::vector<Protocol::ServerUpdate> & game) override {
			
		}
		
		virtual void Update(Protocol::ClientUpdate message) override;
		
		virtual void Connect(std::string address, uint16_t port, std::string hash) override {
			
		}
		
		virtual void Disconnect() override {
			
		}
	};

}
