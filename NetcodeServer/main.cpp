#include <Netcode/DefaultModuleFactory.h>
#include "ServerApp.h"
#include "ProgramOptions.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Json.h>
#include <Netcode/System/SystemClock.h>

int main(int argc, char * argv[]) {
	Netcode::Initialize();
	Netcode::SystemClock::Tick();
	
	Log::Setup(true);
	MainConfig mainConfig;
	
	InitProgramOptions(argc, argv, mainConfig);

	Netcode::IO::Path::SetWorkingDirectiory(Netcode::IO::Path::CurrentWorkingDirectory());
	Netcode::IO::Path::SetShaderRoot(mainConfig.shaderRoot);
	Netcode::IO::Path::SetMediaRoot(mainConfig.mediaRoot);

	Netcode::IO::File configFile{ mainConfig.configFile };

	if(!Netcode::IO::File::Exists(configFile.GetFullPath())) {
		Log::Error("Config file does not exist");
		return 1;
	}

	Netcode::JsonDocument doc;
	Netcode::IO::ParseJsonFromFile(doc, configFile.GetFullPath());
	Netcode::Config::LoadJson(doc);
	
	Netcode::Module::DefaultModuleFactory dmf;
	ServerApp app;
	app.Setup(&dmf);
	app.Run();
	app.Exit();

	return 0;
}
