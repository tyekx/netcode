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

	template<typename ... T>
	void Debug(const char * message, const T & ... values) {
		spdlog::debug(message, values...);
	}

	template<typename ... T>
	void Info(const char * message, const T & ... values) {
		spdlog::info(message, values...);
	}

	template<typename ... T>
	void Warn(const char * message, const T & ... values) {
		spdlog::warn(message, values...);
	}

	template<typename ... T>
	void Error(const char * message, const T & ... values) {
		spdlog::error(message, values...);
	}

	template<typename ... T>
	void Critical(const char * message, const T & ... values) {
		spdlog::critical(message, values...);
	}
	
	template void Debug<std::string>(const char * message, const std::string & value);
	template void Debug<int32_t>(const char * message, const int32_t & value);
	template void Debug<int32_t, int32_t>(const char * message, const int32_t & value, const int32_t & value2);
	template void Debug<uint64_t>(const char * message, const uint64_t & value);
	template void Debug<float, float>(const char * message, const float & value, const float & value2);
	template void Debug<>(const char * message);

	template void Info<>(const char * message);
	template void Info<std::string>(const char * message, const std::string & value);
	template void Info<const char *>(const char * message, const char * const & value);
	template void Info<int32_t>(const char * message, const int32_t & value);
	template void Info<uint16_t>(const char * message, const uint16_t & value);
	template void Info<int32_t, int32_t>(const char * message, const int32_t & value, const int32_t & value2);
	template void Info<uint16_t, uint16_t>(const char * message, const uint16_t & value, const uint16_t & value2);
	template void Info<int32_t, int32_t, int32_t>(const char * message, const int32_t & x, const int32_t & y, const int32_t & z);
	template void Info<uint64_t>(const char * message, const uint64_t & value);

	template void Warn<>(const char * message);

	template void Error<>(const char * message);
	template void Error<std::string>(const char * message, const std::string & value);
	template void Error<const char *>(const char * message, const char * const & value);

	template void Critical<>(const char * message);

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

}
