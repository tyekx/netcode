#include "gtest/gtest.h"
#include <Netcode/URI/Shader.h>
#include <Netcode/URI/Texture.h>
#include <Netcode/URI/Model.h>
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

	EXPECT_EQ(io::Path::GetParentDirectory(L"test/abc/def", L'/'), L"test/abc/");
	EXPECT_EQ(io::Path::GetParentDirectory(L"test/abc/def/", L'/'), L"test/abc/");
	EXPECT_EQ(io::Path::GetParentDirectory(L"/abc", L'/'), L"/");
	EXPECT_EQ(io::Path::GetParentDirectory(L"/", L'/'), L"/");
	EXPECT_EQ(io::Path::GetParentDirectory(L"", L'/'), L"");
	EXPECT_EQ(io::Path::GetParentDirectory(L"C:/", L'/'), L"C:");
	EXPECT_EQ(io::Path::GetParentDirectory(L"arandomfile.jpeg", L'/'), io::Path::WorkingDirectory());
	EXPECT_EQ(io::Path::GetParentDirectory(L"test/abc/config.json", L'/'), L"test/abc/");
}

TEST(UriTest, Texture) {
	namespace io = Netcode::IO;
	namespace u = Netcode::URI;

	const wchar_t * imaginaryTexture = L"image.png";
	u::Texture texture{ imaginaryTexture };
	std::wstring_view mediaRoot = io::Path::MediaRoot();

	std::wstring expectedPath;
	expectedPath.append(mediaRoot);
	expectedPath.append(imaginaryTexture);
	io::Path::FixFilePath(expectedPath);

	EXPECT_FALSE(mediaRoot.empty());
	EXPECT_TRUE(io::Path::IsAbsolute(expectedPath));
	EXPECT_EQ(texture.GetTexturePath(), expectedPath);

	std::wstring parseInput;
	parseInput.append(u::Texture::GetGlobalId());
	parseInput.append(imaginaryTexture);

	u::Texture parsed = u::Texture::Parse(std::move(parseInput));

	EXPECT_EQ(parsed.GetTexturePath(), texture.GetTexturePath());
	EXPECT_EQ(parsed.GetFullPath(), texture.GetFullPath());
	EXPECT_EQ(parsed.GetAssetDirectory(), texture.GetAssetDirectory());
	EXPECT_EQ(parsed.GetAssetDirectory(), io::Path::MediaRoot());

	const wchar_t * imaginaryTex2 = L"textures/white_dot.png";

	u::Texture tex2 = u::Texture{ imaginaryTex2 };
	u::Texture extended = u::Texture::AsExtensionOf(parsed, imaginaryTex2);

	std::wstring expectedExtended;
	expectedExtended.append(u::Texture::GetGlobalId());
	expectedExtended.append(imaginaryTex2);

	EXPECT_EQ(expectedExtended, extended.GetFullPath());
	EXPECT_EQ(tex2.GetFullPath(), extended.GetFullPath());
	EXPECT_EQ(tex2.GetAssetDirectory(), extended.GetAssetDirectory());

	std::wstring secondExpectedExtended;
	secondExpectedExtended.append(u::Texture::GetGlobalId());
	secondExpectedExtended.append(L"textures/");
	secondExpectedExtended.append(imaginaryTex2);

	u::Texture extended2 = u::Texture::AsExtensionOf(extended, imaginaryTex2);

	EXPECT_EQ(extended2.GetFullPath(), secondExpectedExtended);
	
}

TEST(UriTest, Model) {
	namespace io = Netcode::IO;
	namespace u = Netcode::URI;

	const wchar_t * imaginaryModel = L"gun.ncasset";
	u::Model model{ imaginaryModel };
	std::wstring_view mediaRoot = io::Path::MediaRoot();

	std::wstring expectedPath;
	expectedPath.append(mediaRoot);
	expectedPath.append(imaginaryModel);
	io::Path::FixFilePath(expectedPath);

	EXPECT_FALSE(mediaRoot.empty());
	EXPECT_TRUE(io::Path::IsAbsolute(expectedPath));
	EXPECT_EQ(model.GetModelPath(), expectedPath);

	std::wstring parseInput;
	parseInput.append(u::Model::GetGlobalId());
	parseInput.append(imaginaryModel);

	u::Model parsed = u::Model::Parse(std::move(parseInput));

	EXPECT_EQ(parsed.GetModelPath(), model.GetModelPath());
	EXPECT_EQ(parsed.GetFullPath(), model.GetFullPath());
	EXPECT_EQ(parsed.GetAssetDirectory(), model.GetAssetDirectory());
	EXPECT_EQ(parsed.GetAssetDirectory(), io::Path::MediaRoot());
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
	EXPECT_EQ(parsed.GetAssetDirectory(), shader.GetAssetDirectory());
	EXPECT_EQ(parsed.GetAssetDirectory(), io::Path::ShaderRoot());
}


int wmain(int argc, wchar_t * argv[]) {
	std::wstring workingDirectory = Netcode::IO::Path::CurrentWorkingDirectory();
	Netcode::IO::Path::SetWorkingDirectiory(workingDirectory);

	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new Environment{ argc, argv });
	
	return RUN_ALL_TESTS();
}
