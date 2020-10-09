#include <Netcode/Common.h>
#include <Netcode/Utility.h>
#include <chrono>
#include "NetcodeApp.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Json.h>
#include <Netcode/DefaultModuleFactory.h>
#include <NetcodeFoundation/Memory.h>
#include <Netcode/Config.h>
#include "ProgramOptions.h"

#include <dxgi1_3.h>
#include <dxgidebug.h>

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR command, _In_ INT nShowCmd) {
	std::vector<std::wstring> args = po::split_winmain(command);
	std::wstring workingDirectory = Netcode::IO::Path::CurrentWorkingDirectory();

	Netcode::IO::Path::SetWorkingDirectiory(workingDirectory);

	MainConfig mainConfig;
	po::options_description rootDesc("root");

	InitProgramOptions(rootDesc, mainConfig);
	
	po::variables_map vm;

	try {
		po::store(po::wcommand_line_parser(args).options(rootDesc).run(), vm);
		po::notify(vm);
	} catch(po::error & e) {
		Log::Error("Error while parsing command line: {0}", e.what());
		return 1;
	}

	Netcode::IO::Path::SetShaderRoot(mainConfig.shaderRoot);
	Netcode::IO::Path::SetMediaRoot(mainConfig.mediaRoot);

	Netcode::IO::File configFile{ mainConfig.configFile };

	if(!Netcode::IO::File::Exists(configFile.GetFullPath())) {
		Log::Error("File does not exist");
		return 1;
	}

	Netcode::JsonDocument doc;
	Netcode::IO::ParseJsonFromFile(doc, configFile.GetFullPath());
	Netcode::Config::LoadJson(doc);

	Netcode::Input::Initialize();

	Netcode::Module::DefaultModuleFactory defModuleFactory;
	std::unique_ptr<Netcode::Module::AApp> app = std::make_unique<GameApp>();
	app->Setup(&defModuleFactory);

	Log::Info("Initialization successful");

	app->Run();
	app->Exit();
	app.reset();

	Log::Info("Gracefully shutting down");

	IDXGIDebug1 * dxgiDebug;
	if(SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
	{
		dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	}

	return 0;
}
