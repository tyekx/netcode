#pragma once 

#include "NetworkDecl.h"
#include "NetworkCommon.h"
#include "GameSession.h"
#include "MysqlSession.h"
#include "../DestructiveCopyConstructible.hpp"
#include <boost/asio/deadline_timer.hpp>
#include <Netcode/Logger.h>

namespace Netcode::Network {

	class ServerSession : public ServerSessionBase {
		boost::asio::io_context & ioContext;
		Ref<NetcodeService> service;
		
	public:
		virtual ~ServerSession() = default;

		ServerSession(boost::asio::io_context & ioc);

		virtual void Start() override;

		virtual void Stop() override;

		Ref<NetcodeService> GetService() const {
			return service;
		}
	};

}
