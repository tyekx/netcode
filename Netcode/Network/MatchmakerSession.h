#pragma once

#include "NetworkCommon.h"
#include "GameSession.h"

namespace Netcode::Network {

	class MatchmakerSession : public MatchmakerSessionBase {
		boost::asio::io_context & ioContext;
		Ref<NetcodeService> service;
		
	public:
		MatchmakerSession(boost::asio::io_context & ioc) : ioContext{ ioc }, service{} { }

		virtual boost::asio::io_context& GetIOContext() override {
			return ioContext;
		}
		
		void Stop() override;
		
		void Start() override;
		
	};
	
}
