#include <Netcode/Common.h>
#include <Netcode/Utility.h>
#include "NetcodeApp.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Json.h>
#include <Netcode/IO/Directory.h>
#include <Netcode/DefaultModuleFactory.h>
#include <Netcode/Config.h>
#include "ProgramOptions.h"

#include <dxgi1_3.h>
#include <dxgidebug.h>
#include <iostream>
#include <ShlObj.h>


static BOOL AttachOutputToConsole(void) {
	HANDLE consoleHandleOut, consoleHandleError;

	if(AttachConsole(ATTACH_PARENT_PROCESS)) {
		consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if(consoleHandleOut != INVALID_HANDLE_VALUE) {
			freopen("CONOUT$", "w", stdout);
			setvbuf(stdout, NULL, _IONBF, 0);
		} else {
			return FALSE;
		}

		consoleHandleError = GetStdHandle(STD_ERROR_HANDLE);
		if(consoleHandleError != INVALID_HANDLE_VALUE) {
			freopen("CONOUT$", "w", stderr);
			setvbuf(stderr, NULL, _IONBF, 0);
		} else {
			return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

void ExitWithInitializationError(const Netcode::ErrorCode& ec) {
	if(ec) {
		if(AttachOutputToConsole()) {
			std::cout << Netcode::ErrorCodeToString(ec);
			FreeConsole();
		}
	}
	ExitProcess(static_cast<UINT>(ec.value()));
}

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
		std::cout << "Error while parsing command line: " << e.what() << std::endl;
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::BAD_ARGUMENTS));
	}
	
	if(!vm["help"].empty()) {
		std::ostringstream oss;
		oss << rootDesc << std::endl;
		ExitWithInitializationError(Netcode::ErrorCode{});
	}
	
	Netcode::IO::Path::SetShaderRoot(mainConfig.shaderRoot);
	Netcode::IO::Path::SetMediaRoot(mainConfig.mediaRoot);

	Netcode::IO::File configFile{ mainConfig.configFile };

	if(!Netcode::IO::File::Exists(configFile.GetFullPath())) {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::CONFIG_NOT_FOUND));
	}

	if(!Netcode::IO::Directory::Exists(mainConfig.shaderRoot)) {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::SHADER_ROOT_NOT_FOUND));
	}

	if(!Netcode::IO::Directory::Exists(mainConfig.mediaRoot)) {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::MEDIA_ROOT_NOT_FOUND));
	}

	Netcode::JsonDocument doc;
	Netcode::IO::ParseJsonFromFile(doc, configFile.GetFullPath());
	Netcode::Config::LoadJson(doc);

	if(mainConfig.hostMode != L"listen" && mainConfig.hostMode != L"none" && mainConfig.hostMode != L"dedicated") {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::INVALID_NETWORK_MODE));
	}

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
