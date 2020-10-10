#include "ProgramOptions.h"

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
		("network_mode", po::wvalue<std::wstring>(&config.hostMode)->default_value(L"listen", "listen"), "Network mode. Possible values: none, listen, dedicated")
		("public", po::bool_switch(&config.isPublic)->default_value(false), "If set, the application will try to register itself when hosting a game. This value is permanent for dedicated servers. Listen servers can override this value");
	
	root.add(info);
	root.add(paths);
	root.add(netw);
}
