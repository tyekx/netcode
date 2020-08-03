#pragma once

#include <string>

namespace Log {

	void Setup(bool isVerbose);

	template<typename ... T>
	void Debug(const char * message, const T & ... values);
	
	template<typename ... T>
	void Info(const char * message, const T & ... values);

	template<typename ... T>
	void Warn(const char * message, const T & ... values);

	template<typename ... T>
	void Error(const char * message, const T & ... values);

	template<typename ... T>
	void Critical(const char * message, const T & ... values);

	void Debug(int id, const std::string & name, const std::string & password, const std::string & salt, bool isBanned);
	void Debug(int id, int ownerId, int maxPlayers, int interval, int status, const std::string & address, int createdAt, int closedAt);
	void Debug(int id, int userId, const std::string & hash, int expiresAt);
	void Debug(int id, int userId, int gameServerId, int joinedAt, int leftAt);

}


