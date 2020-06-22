#pragma once

#include <cstdint>
#include <string>
#include <optional>

#include <rapidjson/document.h>

#include "../ConfigBase.h"

namespace Netcode::Network {

	struct ProtocolConfig : public ConfigBase {
		/* the amount of time to wait in millisec before the last non acknowledged control message is resent */
		uint32_t resendTimeoutMs;

		/* the amount of time to wait in millisec for an ACK message before reporting an error */
		uint32_t gracePeriodMs;

		/* PPS: packets per second */
		uint32_t maximumAllowedClientPPS;

		ProtocolConfig(Ptree & parentNode) : ConfigBase{ parentNode, "protocol" } { }

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("resendTimeoutMs", resendTimeoutMs, doc.GetAllocator());
			parentObject.AddMember("gracePeriodMs", gracePeriodMs, doc.GetAllocator());
			parentObject.AddMember("maximumAllowedClientPPS", maximumAllowedClientPPS, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			resendTimeoutMs = object["resendTimeoutMs"].GetUint();
			gracePeriodMs = object["gracePeriodMs"].GetUint();
			maximumAllowedClientPPS = object["maximumAllowedClientPPS"].GetUint();

			node->put("resendTimeoutMs", Property{ resendTimeoutMs });
			node->put("gracePeriodMs", Property{ gracePeriodMs });
			node->put("maximumAllowedClientPPS", Property{ maximumAllowedClientPPS });
		}
	};

	struct SessionConfig : public ConfigBase {
		/* time elapsed between ticks in millisec */
		uint32_t tickIntervalMs;
		uint32_t workerThreadCount;

		using ConfigBase::ConfigBase;

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("tickIntervalMs", tickIntervalMs, doc.GetAllocator());
			parentObject.AddMember("workerThreadCount", workerThreadCount, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			tickIntervalMs = object["tickIntervalMs"].GetUint();
			workerThreadCount = object["workerThreadCount"].GetUint();

			node->put("tickIntervalMs", Property{ tickIntervalMs });
			node->put("workerThreadCount", Property{ workerThreadCount });
		}
	};

	struct ServerConfig : public SessionConfig {
		std::string selfAddress;
		std::string hostname;
		uint32_t ownerId;
		/* disconnect users who have not sent a message since this time */
		uint32_t gracePeriodMs;
		uint16_t controlPort;
		uint16_t gamePort;
		uint32_t playerSlots;

		ServerConfig(Ptree & parentNode) : SessionConfig{ parentNode, "server" } {

		}

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

			node->put("selfAddress", Property{ selfAddress });
			node->put("hostname", Property{ hostname });
			node->put("ownerId", Property{ ownerId });
			node->put("gracePeriodMs", Property{ gracePeriodMs });
			node->put("controlPort", Property{ controlPort });
			node->put("gamePort", Property{ gamePort });
			node->put("playerSlots", Property{ playerSlots });
		}
	};

	struct ClientConfig : public SessionConfig {
		std::string hostname;
		uint16_t controlPort;
		uint16_t gamePort;
		uint16_t localPort;

		ClientConfig(Ptree & parentNode) : SessionConfig{ parentNode, "client" } { }

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			SessionConfig::Store(parentObject, doc);
			parentObject.AddMember("hostname", hostname, doc.GetAllocator());
			parentObject.AddMember("controlPort", controlPort, doc.GetAllocator());
			parentObject.AddMember("gamePort", gamePort, doc.GetAllocator());
			parentObject.AddMember("localPort", localPort, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			SessionConfig::Load(object);
			hostname = object["hostname"].GetString();
			controlPort = static_cast<uint16_t>(object["controlPort"].GetUint());
			gamePort = static_cast<uint16_t>(object["gamePort"].GetUint());
			localPort = static_cast<uint16_t>(object["localPort"].GetUint());

			node->put("hostname", Property{ hostname });
			node->put("controlPort", Property{ controlPort });
			node->put("gamePort", Property{ gamePort });
			node->put("localPort", Property{ localPort });
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

	struct WebConfig : public ConfigBase {
		std::string hostname;
		uint16_t port;

		WebConfig(Ptree& parentNode) : ConfigBase{ parentNode, "web" } { }


		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			parentObject.AddMember("hostname", hostname, doc.GetAllocator());
			parentObject.AddMember("port", port, doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			hostname = object["hostname"].GetString();
			port = static_cast<uint16_t>(object["port"].GetUint());
		}
	};

	struct Config : public ConfigBase {
		WebConfig web;
		ProtocolConfig protocol;
		ClientConfig client;
		ServerConfig server;

		Config(Ptree & parentNode) : ConfigBase{ parentNode, "network" }, web{ *node }, protocol{ *node }, client{ *node }, server{ *node } {

		}

		void Store(rapidjson::Value & parentObject, rapidjson::Document & doc) {
			rapidjson::Value wObj{ rapidjson::kObjectType  };
			rapidjson::Value protoObj{ rapidjson::kObjectType };
			rapidjson::Value clientObj{ rapidjson::kObjectType };
			rapidjson::Value serverObj{ rapidjson::kObjectType };

			web.Store(wObj, doc);
			protocol.Store(protoObj, doc);
			client.Store(clientObj, doc);
			server.Store(serverObj, doc);

			parentObject.AddMember("web", wObj.Move(), doc.GetAllocator());
			parentObject.AddMember("protocol", protoObj.Move(), doc.GetAllocator());
			parentObject.AddMember("client", clientObj.Move(), doc.GetAllocator());
			parentObject.AddMember("server", serverObj.Move(), doc.GetAllocator());
		}

		void Load(const rapidjson::Value & object) {
			web.Load(object["web"]);
			protocol.Load(object["protocol"]);
			client.Load(object["client"]);
			server.Load(object["server"]);
		}
	};

}