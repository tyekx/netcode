#include "gtest/gtest.h"
#include <Netcode/URI/Shader.h>
#include <Netcode/IO/Path.h>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Json.h>
#include <Netcode/Config.h>
#include <NetcodeFoundation/Platform.h>
#include <boost/program_options.hpp>

struct MainConfig {
	std::wstring shaderRoot;
	std::wstring mediaRoot;
	std::wstring configFile;
};

namespace po = boost::program_options;

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

	root.add(info);
	root.add(paths);
}

class Environment : public ::testing::Environment {
	MainConfig mc;
public:
	Environment(int argc, wchar_t ** argv) {
		po::options_description desc;

		InitProgramOptions(desc, mc);
		
		po::variables_map vm;

		po::store(po::wcommand_line_parser(argc, argv).options(desc).run(), vm);
		po::notify(vm);
	}

	~Environment() override {}

	void SetUp() override {
		Netcode::IO::Path::SetShaderRoot(mc.shaderRoot);
		Netcode::IO::Path::SetMediaRoot(mc.mediaRoot);

		Netcode::IO::File configFile{ mc.configFile };

		assert(Netcode::IO::File::Exists(configFile.GetFullPath()));

		rapidjson::Document doc;
		Netcode::IO::ParseJson(doc, configFile.GetFullPath());
		Netcode::Config::LoadJson(doc);
	}

	void TearDown() override {

	}
};

TEST(IOTest, Path) {
	namespace io = Netcode::IO;

	EXPECT_TRUE(io::Path::CheckSlashConsistency(L"/abc/def", L'/'));
	EXPECT_FALSE(io::Path::CheckSlashConsistency(L"/abc\\def", L'/'));
	EXPECT_FALSE(io::Path::CheckSlashConsistency(L"/abc\\def", L'\\'));
	EXPECT_TRUE(io::Path::CheckSlashConsistency(L"\\abc\\def", L'\\'));

	wchar_t slash = io::Path::GetSlash();

	EXPECT_TRUE(slash == L'/' || slash == L'\\');

	std::wstring_view src = io::Path::WorkingDirectory();
	
	EXPECT_FALSE(src.empty());
	EXPECT_TRUE(io::Path::CheckSlashConsistency(src, slash));
	EXPECT_TRUE(src.back() == slash);
	EXPECT_TRUE(io::Path::IsAbsolute(src));
	EXPECT_FALSE(io::Path::IsRelative(src));

#if defined(NETCODE_OS_WINDOWS)
	EXPECT_TRUE(io::Path::IsAbsolute(L"C:/work/directx12"));
#endif
	EXPECT_TRUE(io::Path::IsRelative(L"work/directx12"));
	
	{
		std::wstring v = L"/slash\\bslash/f";
		io::Path::FixDirectoryPath(v, L'/');
		EXPECT_EQ(v, L"/slash/bslash/f/");
	}

	{
		std::wstring v = L"/slash\\bslash/f";
		io::Path::FixDirectoryPath(v, L'\\');
		EXPECT_EQ(v, L"\\slash\\bslash\\f\\");
	}

	{
		std::wstring v = L"slash\\bslash/file.c";
		io::Path::FixFilePath(v, L'/');
		EXPECT_EQ(v, L"slash/bslash/file.c");
	}

	{
		std::wstring v = L"slash\\bslash/file.c";
		io::Path::FixFilePath(v, L'\\');
		EXPECT_EQ(v, L"slash\\bslash\\file.c");
	}

}

TEST(UriTest, Shader) {
	namespace io = Netcode::IO;
	namespace u = Netcode::URI;

	const wchar_t * imaginaryShader = L"testShader.hlsl";

	u::Shader shader{ imaginaryShader };
	std::wstring_view shaderRoot = io::Path::ShaderRoot();

	std::wstring expectedPath;
	expectedPath.append(shaderRoot);
	expectedPath.append(imaginaryShader);
	io::Path::FixFilePath(expectedPath);

	EXPECT_FALSE(shaderRoot.empty());
	EXPECT_TRUE(io::Path::IsAbsolute(expectedPath));
	EXPECT_EQ(shader.GetShaderPath(), expectedPath);

	std::wstring parseInput;
	parseInput.append(u::Shader::GetGlobalId());
	parseInput.append(imaginaryShader);

	u::Shader parsed = u::Shader::Parse(std::move(parseInput));

	EXPECT_EQ(parsed.GetShaderPath(), shader.GetShaderPath());
	EXPECT_EQ(parsed.GetFullPath(), shader.GetFullPath());
}


int wmain(int argc, wchar_t * argv[]) {
	std::wstring workingDirectory = Netcode::IO::Path::CurrentWorkingDirectory();
	Netcode::IO::Path::SetWorkingDirectiory(workingDirectory);

	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new Environment{ argc, argv });

	return RUN_ALL_TESTS();
}
