#include <memory>

#include "NetworkCommon.h"

#include <mysqlx/xdevapi.h>

namespace Netcode::Network {

	class DefaultMysqlCompletionHandler {
	public:
		void operator()(ErrorCode errorCode);
	};

	/*
	wrapper for all the mysql statements that will be used during the lifetime of a server
	*/
	class MysqlSession  {
		uint64_t serverId;
		std::unique_ptr<mysqlx::Session> session;
		std::unique_ptr<mysqlx::SqlStatement> queryUserByHash;
		std::unique_ptr<mysqlx::SqlStatement> insertServer;
		std::unique_ptr<mysqlx::SqlStatement> modifyServer;
		std::unique_ptr<mysqlx::SqlStatement> insertGameSession;
		std::unique_ptr<mysqlx::SqlStatement> modifyGameSession;
		std::unique_ptr<mysqlx::SqlStatement> closeRemainingGameSessions;
		std::mutex mutex;
	public:
		MysqlSession() = default;

		ErrorCode QueryUserByHash(const std::string & hash, UserRow & output);

		ErrorCode CloseServer();

		ErrorCode CreateGameSession(int playerId);

		ErrorCode CloseGameSession(int playerId);

		ErrorCode RegisterServer(int ownerId, uint8_t playerSlots, uint32_t tickIntervalMs, const std::string & serverIp, uint16_t controlPort, uint16_t gamePort);

		ErrorCode Connect();
	};

}
