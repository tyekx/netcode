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

	void Info(const char * message) {
		spdlog::info(message);
	}

	void Info(const char * message, const std::string & value) {
		spdlog::info(message, value);
	}

	void Info(const char * message, uint64_t value) {
		spdlog::info(message, value);
	}

	void Warn(const char * message) {
		spdlog::warn(message);
	}

	void Error(const char * message) {
		spdlog::error(message);
	}

	void Critical(const char * message) {
		spdlog::critical(message);
	}

}
