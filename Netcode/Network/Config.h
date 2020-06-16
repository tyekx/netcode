#pragma once

#include <cstdint>
#include <string>

#include <rapidjson/document.h>

namespace Netcode::Network {

	struct ProtocolConfig {
		/* the amount of time to wait in millisec before the last non acknowledged control message is resent */
		uint32_t resendTimeoutMs;

		/* the amount of time to wait in millisec for an ACK message before reporting an error */
		uint32_t gracePeriodMs;

		/* PPS: packets per second */
		uint32_t maximumAllowedClientPPS;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("resendTimeoutMs", resendTimeoutMs, doc.GetAllocator());
			parentObject.AddMember("gracePeriodMs", gracePeriodMs, doc.GetAllocator());
			parentObject.AddMember("maximumAllowedClientPPS", maximumAllowedClientPPS, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			resendTimeoutMs = object["resendTimeoutMs"].GetUint();
			gracePeriodMs = object["gracePeriodMs"].GetUint();
			maximumAllowedClientPPS = object["maximumAllowedClientPPS"].GetUint();
		}
	};

	struct SessionConfig {
		/* time elapsed between ticks in millisec */
		uint32_t tickIntervalMs;
		uint32_t workerThreadCount;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("tickIntervalMs", tickIntervalMs, doc.GetAllocator());
			parentObject.AddMember("workerThreadCount", workerThreadCount, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			tickIntervalMs = object["tickIntervalMs"].GetUint();
			workerThreadCount = object["workerThreadCount"].GetUint();
		}
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


		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			SessionConfig::Store(parentObject, doc);
			parentObject.AddMember("selfAddress", selfAddress, doc.GetAllocator());
			parentObject.AddMember("hostname", hostname, doc.GetAllocator());
			parentObject.AddMember("ownerId", ownerId, doc.GetAllocator());
			parentObject.AddMember("gracePeriodMs", gracePeriodMs, doc.GetAllocator());
			parentObject.AddMember("controlPort", controlPort, doc.GetAllocator());
			parentObject.AddMember("gamePort", gamePort, doc.GetAllocator());
			parentObject.AddMember("playerSlots", playerSlots, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			SessionConfig::Load(object);
			selfAddress = object["selfAddress"].GetString();
			hostname = object["hostname"].GetString();
			ownerId = object["ownerId"].GetUint();
			gracePeriodMs = object["gracePeriodMs"].GetUint();
			controlPort = static_cast<uint16_t>(object["controlPort"].GetUint());
			gamePort = static_cast<uint16_t>(object["gamePort"].GetUint());
			playerSlots = object["playerSlots"].GetUint();
		}
	};

	struct ClientConfig : public SessionConfig {
		std::string serverHost;
		uint16_t serverControlPort;
		uint16_t serverGamePort;
		uint16_t localPort;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			SessionConfig::Store(parentObject, doc);
			parentObject.AddMember("serverHost", serverHost, doc.GetAllocator());
			parentObject.AddMember("serverControlPort", serverControlPort, doc.GetAllocator());
			parentObject.AddMember("serverGamePort", serverGamePort, doc.GetAllocator());
			parentObject.AddMember("localPort", localPort, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			SessionConfig::Load(object);
			serverHost = object["serverHost"].GetString();
			serverControlPort = static_cast<uint16_t>(object["serverControlPort"].GetUint());
			serverGamePort = static_cast<uint16_t>(object["serverGamePort"].GetUint());
			localPort = static_cast<uint16_t>(object["localPort"].GetUint());
		}
	};

	struct DatabaseConfig {
		uint32_t port;
		std::string hostname;
		std::string username;
		std::string password;
		std::string schema;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("port", port, doc.GetAllocator());
			parentObject.AddMember("hostname", hostname, doc.GetAllocator());
			parentObject.AddMember("username", username, doc.GetAllocator());
			parentObject.AddMember("password", password, doc.GetAllocator());
			parentObject.AddMember("schema", schema, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			port = object["port"].GetUint();
			hostname = object["hostname"].GetString();
			username = object["username"].GetString();
			password = object["password"].GetString();
			schema = object["schema"].GetString();
		}
	};

	struct WebConfig {
		std::string hostAddress;
		uint16_t hostPort;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("hostAddress", hostAddress, doc.GetAllocator());
			parentObject.AddMember("hostPort", hostPort, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			hostAddress = object["hostAddress"].GetString();
			hostPort = object["hostPort"].GetUint();
		}
	};

	struct Config {
		WebConfig web;
		ProtocolConfig protocol;
		ClientConfig client;
		ServerConfig server;
		DatabaseConfig database;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			rapidjson::Value wObj{ rapidjson::kObjectType  };
			rapidjson::Value protoObj{ rapidjson::kObjectType };
			rapidjson::Value clientObj{ rapidjson::kObjectType };
			rapidjson::Value serverObj{ rapidjson::kObjectType };
			rapidjson::Value databaseObj{ rapidjson::kObjectType };

			web.Store(wObj, doc);
			protocol.Store(protoObj, doc);
			client.Store(clientObj, doc);
			server.Store(serverObj, doc);
			database.Store(databaseObj, doc);

			parentObject.AddMember("web", wObj.Move(), doc.GetAllocator());
			parentObject.AddMember("protocol", protoObj.Move(), doc.GetAllocator());
			parentObject.AddMember("client", clientObj.Move(), doc.GetAllocator());
			parentObject.AddMember("server", serverObj.Move(), doc.GetAllocator());
			parentObject.AddMember("database", databaseObj.Move(), doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			web.Load(object["web"]);
			protocol.Load(object["protocol"]);
			client.Load(object["client"]);
			server.Load(object["server"]);
			database.Load(object["database"]);
		}
	};

}