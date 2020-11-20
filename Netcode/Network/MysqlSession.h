#pragma once

#include <memory>

#include "NetworkCommon.h"
#include "CompletionToken.h"

namespace Netcode::Network {

	struct QueryUserResult {
		ErrorCode errorCode;
		PlayerDbDataRow playerData;
	};

	class MysqlSession  {

		struct detail;

		std::unique_ptr<detail> impl;
		
	public:
		MysqlSession(boost::asio::io_context& ioc);
		~MysqlSession();

		CompletionToken<QueryUserResult> QueryUserByHash(std::string hash);

		CompletionToken<ErrorCode> CloseServer();

		CompletionToken<ErrorCode> CreateGameSession(int playerId);

		CompletionToken<ErrorCode> CloseGameSession(int playerId);

		CompletionToken<ErrorCode> RegisterServer(int ownerId, uint8_t playerSlots, uint32_t tickIntervalMs, const std::string & serverIp, uint16_t controlPort, uint16_t gamePort);

		CompletionToken<ErrorCode> Connect();
	};

}
