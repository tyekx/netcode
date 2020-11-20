#include "ProgramOptions.h"
#include <NetcodeFoundation/Platform.h>
#include <Netcode/Config.h>
#include "Network/NetwUtil.h"

void InitProgramOptions(po::options_description & root, MainConfig & config) {
	po::options_description info("Information");

	info.add_options()
		("help", "Print help message")
		("version", "Prints version number");

	po::options_description paths("Paths");

	paths.add_options()
		("shader_root", po::wvalue<std::wstring>(&config.shaderRoot)->default_value(L"/", "/"), "Root directory for the compiled shader objects")
		("media_root", po::wvalue<std::wstring>(&config.mediaRoot)->default_value(L"media/", "media/"), "Root directory for the media files")
		("config_file", po::wvalue<std::wstring>(&config.configFile)->default_value(L"config.json", "config.json"), "Configuration file for the program");

	po::options_description netw("Network");

	netw.add_options()
		("host_mode", po::wvalue<std::wstring>(&config.hostMode)->default_value(L"listen", "listen"), "Network host mode. Possible values: client, listen, dedicated")
		("public", po::bool_switch(&config.isPublic)->default_value(false), "If set, the application will try to register itself when hosting a game. This value is permanent for dedicated servers. Listen servers can override this value");

	po::options_description window("Window");

	window.add_options()
		("wnd_pos_x", po::value<int>(&config.windowPosX)->default_value(-1), "Window position X")
		("wnd_pos_y", po::value<int>(&config.windowPosY)->default_value(-1), "Window position Y")
		("wnd_size_x", po::value<int>(&config.windowSizeX)->default_value(-1), "Window size X")
		("wnd_size_y", po::value<int>(&config.windowSizeY)->default_value(-1), "Window size Y");

	po::options_description debugOpts("Debug");

	debugOpts.add_options()
		("dbg_pos_x", po::value<int>(&config.debugPosX)->default_value(-1), "Debug console position X")
		("dbg_pos_y", po::value<int>(&config.debugPosY)->default_value(-1), "Debug console position Y")
		("dbg_size_x", po::value<int>(&config.debugSizeX)->default_value(-1), "Debug console size X")
		("dbg_size_y", po::value<int>(&config.debugSizeY)->default_value(-1), "Debug console size Y")
		("dbg_console", po::value<int>(&config.showDebugConsole)->default_value(-1), "Show debug console (-1): default, (0): no, (1): yes");
	
	root.add(info);
	root.add(paths);
	root.add(netw);
	root.add(window);
	root.add(debugOpts);
}

void LoadVariablesIntoConfig(const MainConfig & config) {
	if(config.hostMode == L"dedicated") {
		Netcode::Config::Set(L"game.hostMode", HostMode::DEDICATED);
	}

	if(config.hostMode == L"listen") {
		Netcode::Config::Set(L"game.hostMode", HostMode::LISTEN);
	}

	if(config.hostMode == L"client") {
		Netcode::Config::Set(L"game.hostMode", HostMode::CLIENT);
	}
	
	if(config.windowSizeX != -1 && config.windowSizeY != -1) {
		Netcode::Config::Set(L"window.size:Int2",
			Netcode::Int2{ config.windowSizeX, config.windowSizeY });
	}

	if(config.windowPosX != -1 && config.windowPosY != -1) {
		Netcode::Config::Set(L"window.position:Int2",
			Netcode::Int2{ config.windowPosX, config.windowPosY });
	}

	if(config.showDebugConsole == 0) {
		Netcode::Config::Set(L"window.debugConsole.enabled:bool", false);
	}

	if(config.showDebugConsole == 1) {
		Netcode::Config::Set(L"window.debugConsole.enabled:bool", true);
	}

	if(config.debugPosX != -1 && config.debugPosY != -1 &&
		config.debugSizeX != -1 && config.debugSizeY != -1) {
		Netcode::Config::Set(L"window.debugConsole.position:Int2", 
			Netcode::Int2{ config.debugPosX, config.debugPosY });
		Netcode::Config::Set(L"window.debugConsole.size:Int2",
			Netcode::Int2{ config.debugSizeX, config.debugSizeY });
		Netcode::Config::Set(L"window.debugConsole.maximized:bool", false);
	}
}
