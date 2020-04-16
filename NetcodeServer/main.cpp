#include <iostream>
#include <NetcodeProtocol/netcode.pb.h>
#include <google/protobuf/io/coded_stream.h>
#include <ServerApp.h>
#include <memory>
#include <Netcode/Network/HttpSession.h>
#include <Netcode/Network/ServerSession.h>
#include <Netcode/DefaultModuleFactory.h>
#include "ProgramOptions.h"

namespace http = boost::beast::http;
namespace net = boost::asio::ip;

int main(int argc, char * argv[]) {
	Log::Setup(true);
	boost::asio::io_context ioc;

	Netcode::Network::Config config;

	po::options_description desc("General Options");
	InitProgramOptions(desc, config);

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::store(po::parse_config_file("config.cfg", desc), vm);
	} catch(po::error& err) {
		Log::Error("Failed to parse configuration: {0}", err.what());
	}
	po::notify(vm);
	
	Netcode::Module::DefaultModuleFactory dmf;

	ServerApp app;
	app.Configure(std::move(config));
	app.Setup(&dmf);
	app.Run();

	return 0;
}
