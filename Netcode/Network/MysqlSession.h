#include <memory>

#include <mysqlx/xdevapi.h>
#include "Config.h"
#include "NetworkCommon.h"

namespace Netcode::Network {
	/*
	wrapper for all the mysql statements that will be used during the lifetime of a server
	*/
	class MysqlSession {
		uint64_t serverId;
		std::unique_ptr<mysqlx::Session> session;
		std::unique_ptr<mysqlx::SqlStatement> queryUserByHash;
		std::unique_ptr<mysqlx::SqlStatement> insertServer;
		std::unique_ptr<mysqlx::SqlStatement> modifyServer;
		std::unique_ptr<mysqlx::SqlStatement> insertGameSession;
		std::unique_ptr<mysqlx::SqlStatement> modifyGameSession;
		std::mutex mutex;
	public:
		MysqlSession() = default;

		ErrorCode QueryUserByHash(const std::string & hash, UserRow & output);

		ErrorCode RegisterServer(const ServerConfig & config);

		ErrorCode CloseServer();

		ErrorCode CreateGameSession(int playerId);

		ErrorCode CloseGameSession(int playerId);

		ErrorCode Connect(const DatabaseConfig & config);

	};

}
