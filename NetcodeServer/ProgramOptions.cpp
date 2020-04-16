#include "ProgramOptions.h"

void InitProgramOptions(po::options_description & root, Netcode::Network::Config & config) {
	root.add_options()
		("version,v", "Prints version string")
		("help", "Prints a help message");
	
	po::options_description database("Database options");
	database.add_options()
		("db.host", po::value<std::string>(&config.database.hostname), "Database host name")
		("db.port", po::value<uint32_t>(&config.database.port)->default_value(33060), "Database port")
		("db.user", po::value<std::string>(&config.database.username), "Database user name")
		("db.pass", po::value<std::string>(&config.database.password), "Database password (unsafe from commandline)")
		("db.schema", po::value<std::string>(&config.database.schema), "Default schema for the database");

	po::options_description server("Server options");
	server.add_options()
		("srv.host", po::value<std::string>(&config.server.hostname)->default_value("localhost"), "The hostname of the server")
		("srv.address", po::value<std::string>(&config.server.selfAddress)->default_value("0.0.0.0"), "The address of the server")
		("srv.owner", po::value<uint32_t>(&config.server.ownerId)->default_value(1), "The creator of this server")
		("srv.interval", po::value<uint32_t>(&config.server.tickIntervalMs)->default_value(500), "The interval in [ms]")
		("srv.port", po::value<uint16_t>(&config.server.controlPort)->default_value(8888), "The control port hint")
		("srv.game_port", po::value<uint16_t>(&config.server.gamePort)->default_value(8889), "The game port hint")
		("srv.grace_period", po::value<uint32_t>(&config.server.gracePeriodMs)->default_value(10000), "Client gets disconnected if not send anything for")
		("srv.slots", po::value<uint32_t>(&config.server.playerSlots)->default_value(8), "Player slots")
		("srv.threads", po::value<uint32_t>(&config.server.workerThreadCount)->default_value(1), "The number of worker threads");

	po::options_description protocol("Protocol options");
	protocol.add_options()
		("proto.grace_period", po::value<uint32_t>(&config.protocol.gracePeriodMs)->default_value(5000), "Protocol ACK grace period")
		("proto.resend", po::value<uint32_t>(&config.protocol.resendTimeoutMs)->default_value(800), "Protocol resend timeout")
		("proto.pps", po::value<uint32_t>(&config.protocol.maximumAllowedClientPPS)->default_value(120), "max. packets per second");

	root.add(protocol);
	root.add(database);
	root.add(server);
}
