#pragma once

#include <string>

namespace Log {

	void Setup(bool isVerbose);

	void Debug(const char * message);
	void Debug(const char * message, int value);
	void Debug(const char * message, int value, int value2);

	void Info(const char * message);
	void Info(const char * message, const std::string & value);
	void Info(const char * message, uint64_t value);
	void Info(const char * message, int32_t x, int32_t y);

	void Warn(const char * message);

	void Error(const char * message);

	void Critical(const char * message);

}


