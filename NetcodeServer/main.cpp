#include <Netcode/DefaultModuleFactory.h>
#include "ServerApp.h"
#include "ProgramOptions.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Json.h>

int main(int argc, char * argv[]) {
	Log::Setup(true);
	MainConfig mainConfig;
	po::options_description rootDesc("General Options");
	
	InitProgramOptions(rootDesc, mainConfig);

	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, rootDesc), vm);
	} catch(po::error& err) {
		Log::Error("Failed to parse configuration: {0}", err.what());
	}
	po::notify(vm);

	Netcode::IO::Path::SetWorkingDirectiory(Netcode::IO::Path::CurrentWorkingDirectory());
	Netcode::IO::Path::SetShaderRoot(mainConfig.shaderRoot);
	Netcode::IO::Path::SetMediaRoot(mainConfig.mediaRoot);

	Netcode::IO::File configFile{ mainConfig.configFile };

	if(!Netcode::IO::File::Exists(configFile.GetFullPath())) {
		Log::Error("Config file does not exist");
		return 1;
	}

	rapidjson::Document doc;
	Netcode::IO::ParseJson(doc, configFile.GetFullPath());
	Netcode::Config::LoadJson(doc);
	
	Netcode::Module::DefaultModuleFactory dmf;
	ServerApp app;
	app.Setup(&dmf);
	app.Run();
	app.Exit();

	return 0;
}
