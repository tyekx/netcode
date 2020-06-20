#pragma once

#include <Netcode/ModulesConfig.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct MainConfig {
	std::wstring shaderRoot;
	std::wstring mediaRoot;
	std::wstring configFile;
};

void InitProgramOptions(po::options_description & root, MainConfig & config);
