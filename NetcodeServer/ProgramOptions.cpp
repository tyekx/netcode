#include "ProgramOptions.h"
#include <boost/program_options.hpp>
#include <Netcode/Logger.h>

namespace po = boost::program_options;

void InitProgramOptions(int argc, char * argv[], MainConfig & config) {
	po::options_description root("General Options");
	po::options_description info("Information");

	info.add_options()
		("help", "Print help message")
		("version", "Prints version number");

	po::options_description paths("Paths");

	paths.add_options()
		("shader_root", po::wvalue<std::wstring>(&config.shaderRoot)->default_value(L"/", "/"), "Root directory for the compiled shader objects")
		("media_root", po::wvalue<std::wstring>(&config.mediaRoot)->default_value(L"media/", "media/"), "Root directory for the media files")
		("config_file", po::wvalue<std::wstring>(&config.configFile)->default_value(L"config.json", "config.json"), "Configuration file for the program");


	root.add(info);
	root.add(paths);
	po::variables_map vm;
	try {
		po::store(po::parse_command_line(argc, argv, root), vm);
	} catch(po::error & err) {
		Log::Error("Failed to parse configuration: {0}", err.what());
	}
	po::notify(vm);
}
