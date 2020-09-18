#include <string>

struct MainConfig {
	std::wstring shaderRoot;
	std::wstring mediaRoot;
	std::wstring configFile;
};

void InitProgramOptions(int argc, char* argv[], MainConfig & config);
