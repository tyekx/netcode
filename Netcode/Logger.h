#pragma once

#include <string>

namespace Log {

	void Setup(bool isVerbose);

	void Debug(const char * message);
	void Debug(const char * message, int value);
	void Debug(const char * message, const std::string& value);
	void Debug(const char * message, int value, int value2);

	void Debug(int id, const std::string & name, const std::string & password, const std::string & salt, bool isBanned);
	void Debug(int id, int ownerId, int maxPlayers, int interval, int status, const std::string & address, int createdAt, int closedAt);
	void Debug(int id, int userId, const std::string & hash, int expiresAt);
	void Debug(int id, int userId, int gameServerId, int joinedAt, int leftAt);

	void Info(const char * message);
	void Info(const char * message, const std::string & value);
	void Info(const char * message, uint64_t value);
	void Info(const char * message, int32_t x, int32_t y);
	void Info(const char * message, int32_t x, int32_t y, int32_t z);

	void Warn(const char * message);

	void Error(const char * message);
	void Error(const char * message, const std::string & value);

	void Critical(const char * message);

}


