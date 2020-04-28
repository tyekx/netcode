#include <Netcode/DefaultModuleFactory.h>
#include <NetcodeFoundation/Version.h>

#include "ProgramOptions.h"
#include "ServerApp.h"

int main(int argc, char * argv[]) {
	Log::Setup(true);
	Log::Info("Build: {0}.{1}.{2}", Netcode::GetMajorVersion(), Netcode::GetMinorVersion(), Netcode::GetBuildVersion());
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
	app.Exit();

	return 0;
}
