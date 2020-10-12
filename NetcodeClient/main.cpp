#include <Netcode/Common.h>
#include <Netcode/Utility.h>
#include "NetcodeApp.h"
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Json.h>
#include <Netcode/IO/Directory.h>
#include <Netcode/DefaultModuleFactory.h>
#include <Netcode/Config.h>
#include <Netcode/System/SecureString.h>
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

namespace ni = Netcode::IO;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR command, _In_ INT nShowCmd) {
	std::vector<std::wstring> args = po::split_winmain(command);
	std::wstring workingDirectory = ni::Path::CurrentWorkingDirectory();

	ni::Path::SetWorkingDirectiory(workingDirectory);

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
	
	ni::Path::SetShaderRoot(mainConfig.shaderRoot);
	ni::Path::SetMediaRoot(mainConfig.mediaRoot);

	ni::File configFile{ mainConfig.configFile };

	if(!ni::File::Exists(configFile.GetFullPath())) {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::CONFIG_NOT_FOUND));
	}

	if(!ni::Directory::Exists(mainConfig.shaderRoot)) {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::SHADER_ROOT_NOT_FOUND));
	}

	if(!ni::Directory::Exists(mainConfig.mediaRoot)) {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::MEDIA_ROOT_NOT_FOUND));
	}

	Netcode::JsonDocument doc;
	ni::ParseJsonFromFile(doc, configFile.GetFullPath());
	Netcode::Config::LoadJson(doc);

	if(mainConfig.hostMode != L"listen" && mainConfig.hostMode != L"none" && mainConfig.hostMode != L"dedicated") {
		ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::INVALID_NETWORK_MODE));
	}

	std::wstring_view appDataDirPath = ni::Path::AppData();

	std::wstring ncAppDataDirPath { appDataDirPath };
	ncAppDataDirPath.append(L"Netcode");

	if(!ni::Directory::Exists(ncAppDataDirPath)) {
		if(!ni::Directory::Create(ncAppDataDirPath)) {
			ExitWithInitializationError(make_error_code(Netcode::ConfigErrc::APPDATA_INACCESSIBLE));
		}
	}
	
	ni::Path::FixDirectoryPath(ncAppDataDirPath);

	ni::File userConfig{ ncAppDataDirPath, L"user.json" };
	ni::File sessionFile{ ncAppDataDirPath, L"session" };

	if(ni::File::Exists(userConfig.GetFullPath())) {
		Netcode::JsonDocument userDoc;
		ni::ParseJsonFromFile(userDoc, userConfig.GetFullPath());
	}

	Netcode::SecureString sessionString;
	
	if(ni::File::Exists(sessionFile.GetFullPath())) {
		ni::FileReader<ni::File> reader{ sessionFile };
		size_t sessionFileSize = reader->GetSize();
		std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(sessionFileSize);
		size_t readBytes = reader->Read(Netcode::MutableArrayView<uint8_t>{ buffer.get(), sessionFileSize });
		sessionString.Store(Netcode::ArrayView<uint8_t>{ buffer.get(), readBytes });
	}

	Netcode::Config::Set<std::wstring>(L"user.sessionFile:string", sessionFile.GetFullPath());
	Netcode::Config::Set<Netcode::SecureString>(L"user.session", sessionString);
	
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
