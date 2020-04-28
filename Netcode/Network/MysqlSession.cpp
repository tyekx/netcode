#include "MysqlSession.h"
#include "../Logger.h"
#include <NetcodeFoundation/Version.h>

namespace Netcode::Network {
	ErrorCode MysqlSession::QueryUserByHash(const std::string & hash, UserRow & output) {
		std::scoped_lock<std::mutex> lock{ mutex };

		try {
			queryUserByHash->bind(hash);
			mysqlx::SqlResult results = queryUserByHash->execute();

			if(results.count() != 1) {
				return Errc::make_error_code(Errc::result_out_of_range);
			}

			mysqlx::Row row = results.fetchOne();

			UserRow userRow;
			userRow.userId = (int)row.get(0);
			userRow.name = (std::string)row.get(1);
			userRow.isBanned = (bool)row.get(2);
			userRow.hash = (std::string)row.get(3);

			output = std::move(userRow);
		} catch(mysqlx::Error & error) {
			Log::Error("[MySQL] QueryUserByHash exception: {0}", error.what());
			return Errc::make_error_code(Errc::host_unreachable);
		}

		return Errc::make_error_code(Errc::success);
	}
	ErrorCode MysqlSession::RegisterServer(const ServerConfig & config) {
		std::scoped_lock<std::mutex> lock{ mutex };
		try {
			// (owner_id, max_players, interval, status, server_ip, control_port, game_port, created_at, major, minor, build)
			insertServer->bind(config.ownerId,
							config.playerSlots,
							config.tickIntervalMs,
							1,
							config.selfAddress,
							config.controlPort,
							config.gamePort,
							time(NULL),
							Netcode::GetMajorVersion(),
							Netcode::GetMinorVersion(),
							Netcode::GetBuildVersion());
			mysqlx::SqlResult res = insertServer->execute();

			if(res.getAffectedItemsCount() != 1) {
				return Errc::make_error_code(Errc::result_out_of_range);
			}

			serverId = res.getAutoIncrementValue();
		} catch(mysqlx::Error & error) {
			Log::Error("[MySQL] RegisterServer exception: {0}", error.what());
			return Errc::make_error_code(Errc::invalid_argument);
		}
		return Errc::make_error_code(Errc::success);
	}
	ErrorCode MysqlSession::CloseServer() {
		std::scoped_lock<std::mutex> lock{ mutex };
		if(serverId == 0) {
			return Errc::make_error_code(Errc::invalid_argument);
		}

		try {
			time_t currentTime = time(NULL);

			modifyServer->bind(2, currentTime, serverId);
			mysqlx::SqlResult result = modifyServer->execute();

			closeRemainingGameSessions->bind(currentTime, serverId);
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
	ErrorCode MysqlSession::CreateGameSession(int playerId) {
		std::scoped_lock<std::mutex> lock{ mutex };

		if(serverId == 0) {
			return Errc::make_error_code(Errc::invalid_argument);
		}

		try {
			// (user_id, game_server_id, joined_at)
			insertGameSession->bind(playerId, serverId, time(NULL));
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
	ErrorCode MysqlSession::CloseGameSession(int playerId) {
		if(serverId == 0) {
			return Errc::make_error_code(Errc::invalid_argument);
		}

		try {
			// UPDATE game_sessions SET left_at = ? WHERE user_id = ? and left_at IS NULL and game_server_id = ?
			modifyGameSession->bind(time(NULL), playerId, serverId);
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
	ErrorCode MysqlSession::Connect(const DatabaseConfig & config) {
		try {
			mysqlx::SessionSettings settings(
				mysqlx::SessionOption::USER, config.username,
				mysqlx::SessionOption::PWD, config.password,
				mysqlx::SessionOption::HOST, config.hostname,
				mysqlx::SessionOption::PORT, config.port,
				mysqlx::SessionOption::DB, config.schema,
				mysqlx::SessionOption::CONNECT_TIMEOUT, std::chrono::seconds(10)
			);

			session = std::make_unique<mysqlx::Session>(settings);

			insertServer = std::make_unique<mysqlx::SqlStatement>(
				session->sql("INSERT INTO game_servers (`owner_id`, `max_players`, `interval`, `status`, `server_ip`, `control_port`, `game_port`, `created_at`, `version_major`, `version_minor`, `version_build`) "
					"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));

			modifyServer = std::make_unique<mysqlx::SqlStatement>(
				session->sql("UPDATE `game_servers` SET `status` = ?, `closed_at` = ? WHERE `id` = ? LIMIT 1"));

			insertGameSession = std::make_unique<mysqlx::SqlStatement>(
				session->sql("INSERT INTO `game_sessions` (`user_id`, `game_server_id`, `joined_at`) "
					"VALUES (?, ?, ?)"));

			modifyGameSession = std::make_unique<mysqlx::SqlStatement>(
				session->sql("UPDATE `game_sessions` SET `left_at` = ? WHERE `user_id` = ? AND `left_at` IS NULL AND `game_server_id` = ?"));

			queryUserByHash = std::make_unique<mysqlx::SqlStatement>(
				session->sql("SELECT `users`.`id`, `users`.`name`, `users`.`is_banned`, `sessions`.`hash`, "
					"IF(`game_sessions`.`user_id` IS NULL OR `game_sessions`.`left_at` IS NOT NULL, 1, 0) AS allowed_to_join "
					"FROM `users` "
					"INNER JOIN `sessions` ON `sessions`.`user_id` = `users`.`id` "
					"LEFT JOIN `game_sessions` ON `game_sessions`.`user_id` = `users`.`id` "
					"WHERE `sessions`.`hash` = ?"
				    "GROUP BY allowed_to_join"));

			closeRemainingGameSessions = std::make_unique<mysqlx::SqlStatement>(
				session->sql("UPDATE `game_sessions` SET `game_sessions`.`left_at` = ? WHERE `game_sessions`.`left_at` IS NULL AND `game_sessions`.`game_server_id` = ?"));

		} catch(mysqlx::Error & error) {
			Log::Error("[MySQL] exception: {0}", error.what());
			return Errc::make_error_code(Errc::host_unreachable);
		}

		return Errc::make_error_code(Errc::success);
	}

	void DefaultMysqlCompletionHandler::operator()(ErrorCode errorCode)
	{
		if(errorCode) {
			Log::Error("[Network] [Database] {0}", errorCode.message());
		}
	}

}
