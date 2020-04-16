#pragma once

#include <cstdint>
#include <string>
#include <boost/asio/ip/address.hpp>

namespace Netcode::Network {

	struct ProtocolConfig {
		/* the amount of time to wait in millisec before the last non acknowledged control message is resent */
		uint32_t resendTimeoutMs;

		/* the amount of time to wait in millisec for an ACK message before reporting an error */
		uint32_t gracePeriodMs;

		/* PPS: packets per second */
		uint32_t maximumAllowedClientPPS;
	};

	struct SessionConfig {
		/* time elapsed between ticks in millisec */
		uint32_t tickIntervalMs;
		uint32_t workerThreadCount;
	};

	struct ServerConfig : public SessionConfig {
		std::string selfAddress;
		std::string hostname;
		/* disconnect users who have not sent a message since this time */
		uint32_t ownerId;
		uint32_t gracePeriodMs;
		uint16_t controlPort;
		uint16_t gamePort;
		uint32_t playerSlots;
	};

	struct ClientConfig : public SessionConfig {
		std::string serverHost;
		uint16_t serverControlPort;
		uint16_t serverGamePort;
		uint16_t localPort;
	};

	struct DatabaseConfig {
		uint32_t port;
		std::string hostname;
		std::string username;
		std::string password;
		std::string schema;
	};

	struct WebConfig {
		std::string hostAddress;
		uint16_t hostPort;
	};

	struct Config {
		WebConfig web;
		ProtocolConfig protocol;
		ClientConfig client;
		ServerConfig server;
		DatabaseConfig database;
	};

}