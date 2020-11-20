#pragma once

#include <Netcode/ModulesConfig.h>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct MainConfig {
	std::wstring shaderRoot;
	std::wstring mediaRoot;
	std::wstring configFile;
	std::wstring hostMode;
	int windowPosX;
	int windowPosY;
	int windowSizeX;
	int windowSizeY;
	int debugPosX;
	int debugPosY;
	int debugSizeX;
	int debugSizeY;
	int showDebugConsole;
	bool isPublic;
};

void InitProgramOptions(po::options_description & root, MainConfig & config);
void LoadVariablesIntoConfig(const MainConfig & config);
