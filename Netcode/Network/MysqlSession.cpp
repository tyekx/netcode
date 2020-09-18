#include "MysqlSession.h"
#include <Netcode/Config.h>
#include <Netcode/Logger.h>
#include <Netcode/Utility.h>
#include <mysqlx/xdevapi.h>


namespace Netcode::Network {

	struct MysqlSession::detail {
		uint64_t serverId;
		std::unique_ptr<mysqlx::Session> session;
		std::unique_ptr<mysqlx::SqlStatement> queryUserByHash;
		std::unique_ptr<mysqlx::SqlStatement> insertServer;
		std::unique_ptr<mysqlx::SqlStatement> modifyServer;
		std::unique_ptr<mysqlx::SqlStatement> insertGameSession;
		std::unique_ptr<mysqlx::SqlStatement> modifyGameSession;
		std::unique_ptr<mysqlx::SqlStatement> closeRemainingGameSessions;
		std::unique_ptr<mysqlx::SqlStatement> debugCleanup;
		std::mutex mutex;

		ErrorCode QueryUserByHash(const std::string & hash, PlayerDbDataRow & output) {
			std::scoped_lock<std::mutex> lock{ mutex };
			try {
				queryUserByHash->bind(hash);
				mysqlx::SqlResult results = queryUserByHash->execute();

				if(results.count() != 1) {
					return Errc::make_error_code(Errc::result_out_of_range);
				}

				mysqlx::Row row = results.fetchOne();

				int userId = (int)row.get(0);
				std::string name = (std::string)row.get(1);
				bool isBanned = (bool)row.get(2);
				std::string hash = (std::string)row.get(3);

				output = std::tie(userId, name, hash, isBanned);
			} catch(mysqlx::Error & error) {
				Log::Error("[MySQL] QueryUserByHash exception: {0}", error.what());
				return Errc::make_error_code(Errc::host_unreachable);
			}
			return Errc::make_error_code(Errc::success);
		}
		
		ErrorCode CloseServer() {
			std::scoped_lock<std::mutex> lock{ mutex };
			if(serverId == 0) {
				return Errc::make_error_code(Errc::invalid_argument);
			}

			try {
				modifyServer->bind(2, serverId);
				mysqlx::SqlResult result = modifyServer->execute();

				closeRemainingGameSessions->bind(serverId);
				mysqlx::SqlResult closeResult = closeRemainingGameSessions->execute();

				if(result.getAffectedItemsCount() != 1) {
					return Errc::make_error_code(Errc::result_out_of_range);
				}
			} catch(mysqlx::Error & error) {
				Log::Error("[MySQL] CloseServer exception: {0}", error.what());
				return Errc::make_error_code(Errc::invalid_argument);
			}
			return Errc::make_error_code(Errc::success);
		}
		ErrorCode CreateGameSession(int playerId) {
			std::scoped_lock<std::mutex> lock{ mutex };

			if(serverId == 0) {
				return Errc::make_error_code(Errc::invalid_argument);
			}

			try {
				// (user_id, game_server_id)
				insertGameSession->bind(playerId, serverId);
				mysqlx::SqlResult result = insertGameSession->execute();

				if(result.getAffectedItemsCount() != 1) {
					return Errc::make_error_code(Errc::result_out_of_range);
				}
			} catch(mysqlx::Error & error) {
				Log::Error("[MySQL] CreateGameSession exception: {0}", error.what());
				return Errc::make_error_code(Errc::invalid_argument);
			}
			return Errc::make_error_code(Errc::success);
		}
		ErrorCode CloseGameSession(int playerId) {
			if(serverId == 0) {
				return Errc::make_error_code(Errc::invalid_argument);
			}

			try {
				modifyGameSession->bind(playerId, serverId);
				mysqlx::SqlResult result = modifyGameSession->execute();

				if(result.getAffectedItemsCount() != 1) {
					return Errc::make_error_code(Errc::result_out_of_range);
				}
			} catch(mysqlx::Error & error) {
				Log::Error("[MySQL] CloseGameSession exception: {0}", error.what());
				return Errc::make_error_code(Errc::invalid_argument);
			}
			return Errc::make_error_code(Errc::success);
		}

		ErrorCode RegisterServer(int ownerId, uint8_t playerSlots, uint32_t tickIntervalMs, const std::string & serverIp, uint16_t controlPort, uint16_t gamePort)
		{
			std::scoped_lock<std::mutex> lock{ mutex };

			try {
				// (owner_id, max_players, interval, status, server_ip, control_port, game_port, major, minor, build)
				insertServer->bind(ownerId,
					playerSlots,
					tickIntervalMs,
					1,
					serverIp,
					controlPort,
					gamePort,
					0, 0, 0);
				mysqlx::SqlResult res = insertServer->execute();

				if(res.getAffectedItemsCount() != 1) {
					return Errc::make_error_code(Errc::result_out_of_range);
				}

				serverId = res.getAutoIncrementValue();

#if defined(NETCODE_DEBUG)
				debugCleanup->execute();
#endif
				
			} catch(mysqlx::Error & error) {
				Log::Error("[MySQL] RegisterServer exception: {0}", error.what());
				return Errc::make_error_code(Errc::invalid_argument);
			}
			return Errc::make_error_code(Errc::success);
		}

		ErrorCode Connect()
		{
			try {
				mysqlx::SessionSettings settings(
					mysqlx::SessionOption::USER, Config::Get<std::wstring>(L"network.database.username:string"),
					mysqlx::SessionOption::PWD, Config::Get<std::wstring>(L"network.database.password:string"),
					mysqlx::SessionOption::HOST, Config::Get<std::wstring>(L"network.database.hostname:string"),
					mysqlx::SessionOption::PORT, Config::Get<uint16_t>(L"network.database.port:u16"),
					mysqlx::SessionOption::DB, Config::Get<std::wstring>(L"network.database.schema:string"),
					mysqlx::SessionOption::CONNECT_TIMEOUT, std::chrono::seconds(Config::Get<uint32_t>(L"network.database.timeout:u32"))
				);

				session = std::make_unique<mysqlx::Session>(settings);

				insertServer = std::make_unique<mysqlx::SqlStatement>(
					session->sql("INSERT INTO game_servers (`owner_id`, `max_players`, `interval`, `status`, `server_ip`, `control_port`, `game_port`, `created_at`, `version_major`, `version_minor`, `version_build`) "
						"VALUES (?, ?, ?, ?, ?, ?, ?, NOW(6), ?, ?, ?)"));

				modifyServer = std::make_unique<mysqlx::SqlStatement>(
					session->sql("UPDATE `game_servers` SET `status` = ?, `closed_at` = NOW(6) WHERE `id` = ? LIMIT 1"));

				insertGameSession = std::make_unique<mysqlx::SqlStatement>(
					session->sql("INSERT INTO `game_sessions` (`user_id`, `game_server_id`, `joined_at`) "
						"VALUES (?, ?, NOW(6))"));

				modifyGameSession = std::make_unique<mysqlx::SqlStatement>(
					session->sql("UPDATE `game_sessions` SET `left_at` = NOW(6) WHERE `user_id` = ? AND `left_at` = 0 AND `game_server_id` = ?"));

				queryUserByHash = std::make_unique<mysqlx::SqlStatement>(
					session->sql("SELECT `users`.`id`, `users`.`name`, `users`.`is_banned`, `sessions`.`hash` "
						"FROM `users` "
						"INNER JOIN `sessions` ON `sessions`.`user_id` = `users`.`id` "
						"WHERE `sessions`.`hash` = ? AND `sessions`.`expires_at` > NOW(6)"));

				closeRemainingGameSessions = std::make_unique<mysqlx::SqlStatement>(
					session->sql("UPDATE `game_sessions` SET `game_sessions`.`left_at` = NOW(6) WHERE `game_sessions`.`left_at` = 0 AND `game_sessions`.`game_server_id` = ?"));

#if defined (NETCODE_DEBUG)
				debugCleanup = std::make_unique<mysqlx::SqlStatement>(
					session->sql("UPDATE `game_sessions` SET `game_sessions`.`left_at` = NOW(6) WHERE `game_sessions`.`left_at` = 0"));
#endif

			} catch(mysqlx::Error & error) {
				Log::Error("[MySQL] exception: {0}", error.what());
				return Errc::make_error_code(Errc::host_unreachable);
			}
			return Errc::make_error_code(Errc::success);
		}
	};
	

	void DefaultMysqlCompletionHandler::operator()(ErrorCode errorCode)
	{
		if(errorCode) {
			Log::Error("[Network] [Database] {0}", errorCode.message());
		}
	}

	MysqlSession::MysqlSession()
	{
		impl = std::make_unique<detail>();
	}

	MysqlSession::~MysqlSession()
	{
		impl.reset();
	}

	ErrorCode MysqlSession::QueryUserByHash(const std::string& hash, PlayerDbDataRow & output)
	{
		return impl->QueryUserByHash(hash, output);
	}

	ErrorCode MysqlSession::CloseServer()
	{
		return impl->CloseServer();
	}

	ErrorCode MysqlSession::CreateGameSession(int playerId)
	{
		return impl->CreateGameSession(playerId);
	}

	ErrorCode MysqlSession::CloseGameSession(int playerId)
	{
		return impl->CloseGameSession(playerId);
	}
	
	ErrorCode MysqlSession::RegisterServer(int ownerId, uint8_t playerSlots, uint32_t tickIntervalMs, const std::string & serverIp, uint16_t controlPort, uint16_t gamePort)
	{
		return impl->RegisterServer(ownerId, playerSlots, tickIntervalMs, serverIp, controlPort, gamePort);
	}
	
	ErrorCode MysqlSession::Connect()
	{
		return impl->Connect();
	}
}
