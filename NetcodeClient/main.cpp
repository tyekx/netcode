#include <Netcode/Common.h>
#include <Netcode/Utility.h>
#include <chrono>
#include "NetcodeApp.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <shellapi.h>
#include <Netcode/DefaultModuleFactory.h>
#include <NetcodeFoundation/Memory.h>

#include "ProgramOptions.h"
/*
*/

std::wstring QueryWorkingDirectory() {
	DWORD len = GetCurrentDirectoryW(0, nullptr);
	std::wstring pwd;
	pwd.resize(len);
	GetCurrentDirectoryW(static_cast<DWORD>(pwd.size()), pwd.data());
	pwd.resize(len - 1);
	return pwd;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR command, _In_ INT nShowCmd) {

	std::vector<std::wstring> args = po::split_winmain(command);
	std::wstring workingDirectory = QueryWorkingDirectory();

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

	if(!configFile.Open(Netcode::IO::FileOpenMode::READ)) {
		Log::Error("Failed to open config file");
		return 1;
	}

	size_t numBytes = configFile.GetSize();
	void * buffer = std::malloc(numBytes);
	configFile.Read(Netcode::MutableArrayView<uint8_t>(static_cast<uint8_t *>(buffer), numBytes));
	configFile.Close();

	rapidjson::Document doc{ };
	rapidjson::MemoryStream ms{ static_cast<char*>(buffer), numBytes };
	doc.ParseStream(ms);
	std::free(buffer);

	Netcode::Config configuration;
	configuration.Load(doc);

	Netcode::Input::CreateResources();

	Netcode::Module::DefaultModuleFactory defModuleFactory;
	std::unique_ptr<Netcode::Module::AApp> app = std::make_unique<GameApp>();
	app->Setup(&defModuleFactory, configuration);

	Log::Info("Initialization successful");

	app->Run();
	app->Exit();

	Log::Info("Gracefully shutting down");

	return 0;
}
