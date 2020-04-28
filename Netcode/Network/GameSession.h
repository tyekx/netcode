#pragma once 

#include <NetcodeProtocol/netcode.pb.h>

#include "Config.h"
#include "NetworkCommon.h"

namespace Netcode::Network {

	class GameSession : public std::enable_shared_from_this<GameSession> {
	public:
		virtual ~GameSession() = default;

		virtual bool CheckVersion(const Netcode::Protocol::Version & version) = 0;
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual void SendAll() = 0;
		virtual void Receive(std::vector<Protocol::Message> & control, std::vector<Protocol::Message> & game) = 0;
		virtual void SendUpdate(const udp_endpoint_t & endpoint, Protocol::Message message) = 0;
		virtual void SendUpdate(Protocol::Message message) = 0;
		virtual void SendControlMessage(Netcode::Protocol::Message message, std::function<void(ErrorCode)> completionHandler) = 0;
		virtual bool IsRunning() const = 0;
		virtual std::string GetLastError() const = 0;
		
	};

	using GameSessionRef = std::shared_ptr<GameSession>;
	/*
	class NetworkModule {
		boost::asio::io_context ioc;
		boost::asio::ip::udp::resolver udpResolver;
		
	public:
		virtual void Start() = 0;
		virtual void Shutdown() = 0;

		virtual GameSessionRef CreateServer(SessionConfig config) = 0;
		virtual GameSessionRef CreateClient(SessionConfig config) = 0;
	};*/

}
