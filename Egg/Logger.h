#pragma once

#include <string>

namespace Log {

	void Setup(bool isVerbose);

	void Debug(const char * message);
	void Debug(const char * message, int value);

	void Info(const char * message);
	void Info(const char * message, const std::string & value);

	void Warn(const char * message);

	void Error(const char * message);

	void Critical(const char * message);

}


