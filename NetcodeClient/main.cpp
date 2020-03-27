#include <Netcode/Common.h>
#include <Netcode/Utility.h>
#include <chrono>
#include "NetcodeApp.h"
#include <Netcode/ProgramArgs.h>
#include <Netcode/Path.h>
#include <shellapi.h>
#include <Netcode/DefaultModuleFactory.h>
/*
*/

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR command, _In_ INT nShowCmd) {
	 
	int argc;
	wchar_t ** args = CommandLineToArgvW(command, &argc);
	
	Netcode::ProgramArgs pa{ (const wchar_t**)args, argc };

	LocalFree(args);

	if(!pa.IsSet(L"shaderPath") || !pa.IsSet(L"mediaPath")) {
		Log::Error("Missing command arguments");
		return 1;
	}

	Netcode::Path::SetShaderRoot(pa.GetArg(L"shaderPath"));
	Netcode::Path::SetMediaRoot(pa.GetArg(L"mediaPath"));

	Netcode::Input::CreateResources();

	Netcode::Module::DefaultModuleFactory defModuleFactory;
	std::unique_ptr<Netcode::Module::AApp> app = std::make_unique<GameApp>();
	app->Setup(&defModuleFactory);

	Log::Info("Initialization successful");

	app->Run();
	app->Exit();

	Log::Info("Gracefully shutting down");

	return 0;
}
