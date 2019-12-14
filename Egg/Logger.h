#pragma once

/*
compile times became a joke after including spdlog, so the templates must be "specialized" in the Logger.cpp file. That way it does not leak into the other source files
Spdlog does not follow the C-style formatStrings. Consult with the documentation online
*/

namespace Log {

	void Setup(bool isVerbose);

	void Debug(const char * message);

	void Info(const char * message);

	void Warn(const char * message);

	void Error(const char * message);

	void Critical(const char * message);

}


