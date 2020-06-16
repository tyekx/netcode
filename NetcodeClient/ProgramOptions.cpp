#include "ProgramOptions.h"

void InitProgramOptions(po::options_description & root, Netcode::Config & config) {
	po::options_description web("Web options");
	web.add_options()
		("netcode.web.host", po::value<std::string>(&config.network.web.hostAddress)->default_value("http://netcode.webs"), "The hostname of the web host")
		("netcode.web.port", po::value<uint16_t>(&config.network.web.hostPort)->default_value(80), "The port of the web host");

	po::options_description protocol("Protocol options");
	protocol.add_options()
		("netcode.proto.grace_period", po::value<uint32_t>(&config.network.protocol.gracePeriodMs)->default_value(5000), "Protocol ACK grace period")
		("netcode.proto.resend", po::value<uint32_t>(&config.network.protocol.resendTimeoutMs)->default_value(800), "Protocol resend timeout")
		("netcode.proto.max_pps", po::value<uint32_t>(&config.network.protocol.maximumAllowedClientPPS)->default_value(120), "max. packets per second");
}
