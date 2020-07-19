#include "Logger.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Log {

	

	void Setup(bool isVerbose) {
		auto console = spdlog::stdout_color_mt("console");

		if(isVerbose) {
			spdlog::set_level(spdlog::level::debug);
		} else {
			spdlog::set_level(spdlog::level::info);
		}

		/*
		%T: HH:MM:SS
		%P: pid
		%t: thread id
		%l: log level
		%v: message
		*/
		//spdlog::set_pattern("[%T][%P][%t][%l] %v");
		spdlog::set_default_logger(console);
	}

	void Debug(const char * message) {
		spdlog::debug(message);
	}

	void Debug(const char * message, int value)
	{
		spdlog::debug(message, value);
	}

	void Debug(const char * message, const std::string & value)
	{
		spdlog::debug(message, value);
	}

	void Debug(const char * message, int value, int value2)
	{
		spdlog::debug(message, value, value2);
	}

	void Debug(const char * message, float x, float y)
	{
		spdlog::debug(message, x, y);
	}

	void Debug(int id, const std::string & name, const std::string & password, const std::string & salt, bool isBanned)
	{
		spdlog::debug("[Db][users] id: {0}, name: {1}, password: {2}, salt: {3}, is_banned: {4}", id, name, password, salt, isBanned);
	}

	void Debug(int id, int ownerId, int maxPlayers, int interval, int status, const std::string & address, int createdAt, int closedAt)
	{
		spdlog::debug("[Db][game_servers] id: {0}, owner_id: {1}, max_players: {2}, interval: {3}, status: {4}, server_ip: {5}, created_at: {6}, closed_at: {7}",
				id, ownerId, maxPlayers, interval, status, address, createdAt, closedAt);
	}

	void Debug(int id, int userId, const std::string & hash, int expiresAt)
	{
		spdlog::debug("[Db][sessions] id: {0}, user_id: {1}, hash: {2}, expires_at: {3}", id, userId, hash, expiresAt);
	}

	void Debug(int id, int userId, int gameServerId, int joinedAt, int leftAt)
	{
		spdlog::debug("[Db][game_sessions] id: {0}, user_id: {1}, game_server_id: {2}, joined_at: {3}, left_at: {4}",
			id, userId, gameServerId, joinedAt, leftAt);
	}

	void Info(const char * message) {
		spdlog::info(message);
	}

	void Info(const char * message, const std::string & value) {
		spdlog::info(message, value);
	}

	void Info(const char * message, uint64_t value) {
		spdlog::info(message, value);
	}

	void Info(const char * message, int32_t x, int32_t y) {
		spdlog::info(message, x, y);
	}

	void Info(const char * message, int32_t x, int32_t y, int32_t z)
	{
		spdlog::info(message, x, y, z);
	}

	void Warn(const char * message) {
		spdlog::warn(message);
	}

	void Error(const char * message) {
		spdlog::error(message);
	}

	void Error(const char * message, const std::string & value)
	{
		spdlog::error(message, value);
	}

	void Critical(const char * message) {
		spdlog::critical(message);
	}

}
