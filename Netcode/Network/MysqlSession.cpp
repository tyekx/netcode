#include "MysqlSession.h"
#include <Netcode/Config.h>
#include <Netcode/Logger.h>
#include <Netcode/Utility.h>
#include <mysqlx/xdevapi.h>


namespace Netcode::Network {

	struct MysqlSession::detail {
		boost::asio::io_context & ioContext;
		boost::asio::strand<boost::asio::io_context::executor_type> strand;
		uint64_t serverId;
		std::unique_ptr<mysqlx::Session> session;
		std::unique_ptr<mysqlx::SqlStatement> queryUserByHash;
		std::unique_ptr<mysqlx::SqlStatement> insertServer;
		std::unique_ptr<mysqlx::SqlStatement> modifyServer;
		std::unique_ptr<mysqlx::SqlStatement> insertGameSession;
		std::unique_ptr<mysqlx::SqlStatement> modifyGameSession;
		std::unique_ptr<mysqlx::SqlStatement> closeRemainingGameSessions;
		std::unique_ptr<mysqlx::SqlStatement> debugCleanup;

		detail(boost::asio::io_context & ioc) :
			ioContext{ ioc },
			strand{ boost::asio::make_strand(ioc) },
			serverId{} {
			
		}

		CompletionToken<QueryUserResult> QueryUserByHash(std::string hash) {
			CompletionToken<QueryUserResult> token = std::make_shared<CompletionTokenType<QueryUserResult>>(&ioContext);

			boost::asio::post(strand, [this, h = std::move(hash), token]() mutable {
				token->Set(QueryUserByHashUnsafe(std::move(h)));
			});

			return token;
		}
		
		QueryUserResult QueryUserByHashUnsafe(std::string hash) {
			QueryUserResult result;
			try {
				queryUserByHash->bind(hash);
				mysqlx::SqlResult results = queryUserByHash->execute();

				if(results.count() != 1) {
					result.errorCode = Errc::make_error_code(Errc::result_out_of_range);
					return result;
				}

				mysqlx::Row row = results.fetchOne();

				int userId = (int)row.get(0);
				std::string name = (std::string)row.get(1);
				bool isBanned = (bool)row.get(2);
				std::string hash = (std::string)row.get(3);

				result.playerData = std::tie(userId, name, hash, isBanned);
			} catch(mysqlx::Error & error) {
				Log::Error("[MySQL] QueryUserByHash exception: {0}", error.what());
				result.errorCode = Errc::make_error_code(Errc::host_unreachable);
				return result;
			}
			result.errorCode = Errc::make_error_code(Errc::success);
			return result;
		}

		CompletionToken<ErrorCode> CloseServer() {
			CompletionToken<ErrorCode> token = std::make_shared<CompletionTokenType<ErrorCode>>(&ioContext);

			boost::asio::post(strand, [this, token]() mutable {
				token->Set(CloseServerUnsafe());
			});

			return token;
		}
		
		ErrorCode CloseServerUnsafe() {
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
		
		ErrorCode CreateGameSessionUnsafe(int playerId) {
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
		ErrorCode CloseGameSessionUnsafe(int playerId) {
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

		ErrorCode RegisterServerUnsafe(int ownerId, uint8_t playerSlots, uint32_t tickIntervalMs, std::string serverIp, uint16_t controlPort, uint16_t gamePort)
		{

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

		ErrorCode ConnectUnsafe()
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

				/*
				 * select users.id, users.name, game_servers.owner_id, SUM(IF(game_servers.closed_at is null and owner_id is not null, 1, 0))
				 * as test from users inner join sessions on users.id = sessions.user_id left join game_servers on users.id = game_servers.owner_id
				 * group by users.id
				 */

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

	MysqlSession::MysqlSession(boost::asio::io_context& ioc)
	{
		impl = std::make_unique<detail>(ioc);
	}

	MysqlSession::~MysqlSession()
	{
		impl.reset();
	}

	CompletionToken<QueryUserResult> MysqlSession::QueryUserByHash(std::string hash)
	{
		return impl->QueryUserByHash(std::move(hash));
	}

	CompletionToken<ErrorCode> MysqlSession::CloseServer()
	{
		return impl->CloseServer();
	}

	CompletionToken<ErrorCode> MysqlSession::CreateGameSession(int playerId)
	{
		//return impl->CreateGameSession(playerId);
		return nullptr;
	}

	CompletionToken<ErrorCode> MysqlSession::CloseGameSession(int playerId)
	{
		//return impl->CloseGameSession(playerId);
		return nullptr;
	}
	
	CompletionToken<ErrorCode> MysqlSession::RegisterServer(int ownerId, uint8_t playerSlots, uint32_t tickIntervalMs, const std::string & serverIp, uint16_t controlPort, uint16_t gamePort)
	{
		//return impl->RegisterServer(ownerId, playerSlots, tickIntervalMs, serverIp, controlPort, gamePort);
		return nullptr;
	}
	
	CompletionToken<ErrorCode> MysqlSession::Connect()
	{
		//return impl->Connect();
		return nullptr;
	}
}
