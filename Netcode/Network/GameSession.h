#pragma once 

#include <NetcodeProtocol/netcode.pb.h>

#include "Config.h"
#include "NetworkCommon.h"

namespace Netcode::Network {

	class GameSession {
	public:
		virtual ~GameSession() = default;

		virtual void Receive(std::vector<Protocol::Message> & control, std::vector<Protocol::Message> & game) = 0;
		virtual void SendUpdate(const udp_endpoint_t & endpoint, Protocol::Message message) = 0;
		virtual void SendUpdate(Protocol::Message message) = 0;
		virtual std::future<ErrorCode> SendControlMessage(Netcode::Protocol::Message message) = 0;
		virtual bool IsRunning() const = 0;
		virtual std::string GetLastError() const = 0;
		/*
		virtual void SetPlayerState(Netcode::Protocol::User user) = 0;
		virtual Netcode::Protocol::User * GetPlayerState(int32_t userId) = 0;

		virtual void AddText(Netcode::Protocol::Text text) = 0;
		virtual void AddShot(Netcode::Protocol::Shot shot) = 0;*/
		
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
