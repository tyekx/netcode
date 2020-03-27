#include <Netcode/Common.h>
#include <Netcode/Utility.h>
#include <chrono>
#include "EggApp.h"
#include <Netcode/ProgramArgs.h>
#include <Netcode/Path.h>
#include <shellapi.h>
#include <Netcode/DefaultModuleFactory.h>
/*
*/

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR command, _In_ INT nShowCmd) {
	 
	int argc;
	wchar_t ** args = CommandLineToArgvW(command, &argc);
	
	Egg::ProgramArgs pa{ (const wchar_t**)args, argc };

	LocalFree(args);

	if(!pa.IsSet(L"shaderPath") || !pa.IsSet(L"mediaPath")) {
		Log::Error("Missing command arguments");
		return 1;
	}

	Egg::Path::SetShaderRoot(pa.GetArg(L"shaderPath"));
	Egg::Path::SetMediaRoot(pa.GetArg(L"mediaPath"));

	Egg::Input::CreateResources();

	Egg::Module::DefaultModuleFactory defModuleFactory;
	std::unique_ptr<Egg::Module::AApp> app = std::make_unique<GameApp>();
	app->Setup(&defModuleFactory);

	Log::Info("Initialization successful");

	app->Run();
	app->Exit();

	Log::Info("Gracefully shutting down");

	return 0;
}
