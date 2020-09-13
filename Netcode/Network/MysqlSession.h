#include <memory>

#include "NetworkCommon.h"

namespace Netcode::Network {

	class DefaultMysqlCompletionHandler {
	public:
		void operator()(ErrorCode errorCode);
	};

	/*
	wrapper for all the mysql statements that will be used during the lifetime of a server,
	must hide implementation detail because mysqlx library defines "INTERNAL" as a macro and
	it collides with gRPC.
	*/
	class MysqlSession  {

		struct detail;

		std::unique_ptr<detail> impl;
		
	public:
		MysqlSession();
		~MysqlSession();

		ErrorCode QueryUserByHash(const std::string & hash, PlayerDbDataRow & output);

		ErrorCode CloseServer();

		ErrorCode CreateGameSession(int playerId);

		ErrorCode CloseGameSession(int playerId);

		ErrorCode RegisterServer(int ownerId, uint8_t playerSlots, uint32_t tickIntervalMs, const std::string & serverIp, uint16_t controlPort, uint16_t gamePort);

		ErrorCode Connect();
	};

}
