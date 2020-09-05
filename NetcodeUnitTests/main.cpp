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
#include <NetcodeFoundation/Json.h>
#include <Netcode/Network/ReplicationContext.h>

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

TEST(Replication, Conversion) {
	Netcode::JsonDocument doc;
	doc.SetObject();

	Netcode::JsonValue v;
	Netcode::JsonValueConverter<>::ConvertToJson(-10, v, doc.GetAllocator());
	EXPECT_TRUE(v.IsInt());
	
	Netcode::JsonValueConverter<>::ConvertToJson(10u, v, doc.GetAllocator());
	EXPECT_TRUE(v.IsUint());

	Netcode::JsonValueConverter<>::ConvertToJson(10.0f, v, doc.GetAllocator());
	EXPECT_TRUE(v.IsFloat());

	Netcode::JsonValueConverter<>::ConvertToJson(10.0, v, doc.GetAllocator());
	EXPECT_TRUE(v.IsDouble());

	{
		Netcode::Float4 f4Input{ 1.0f, 2.0f, 3.0f, 4.0f };
		Netcode::Float4 f4Output;

		Netcode::JsonValueConverter<>::ConvertToJson(f4Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 4);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, f4Output);
		EXPECT_TRUE(f4Input.x == f4Output.x);
		EXPECT_TRUE(f4Input.y == f4Output.y);
		EXPECT_TRUE(f4Input.z == f4Output.z);
		EXPECT_TRUE(f4Input.w == f4Output.w);
	}

	{
		Netcode::Float3 f3Input{ 5.0f, 6.0f, 7.0f };
		Netcode::Float3 f3Output;

		Netcode::JsonValueConverter<>::ConvertToJson(f3Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 3);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, f3Output);
		EXPECT_TRUE(f3Input.x == f3Output.x);
		EXPECT_TRUE(f3Input.y == f3Output.y);
		EXPECT_TRUE(f3Input.z == f3Output.z);
	}

	{
		Netcode::Float2 f2Input{ 8.0f, 9.0f };
		Netcode::Float2 f2Output;

		Netcode::JsonValueConverter<>::ConvertToJson(f2Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 2);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, f2Output);
		EXPECT_TRUE(f2Input.x == f2Output.x);
		EXPECT_TRUE(f2Input.y == f2Output.y);
	}

	{
		Netcode::Int4 i4Input{ -1,-2,-3,-4 };
		Netcode::Int4 i4Output;

		Netcode::JsonValueConverter<>::ConvertToJson(i4Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 4);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, i4Output);
		EXPECT_TRUE(i4Input.x == i4Output.x);
		EXPECT_TRUE(i4Input.y == i4Output.y);
		EXPECT_TRUE(i4Input.z == i4Output.z);
		EXPECT_TRUE(i4Input.w == i4Output.w);
	}

	{
		Netcode::Int3 i3Input{ -5,-6,-7 };
		Netcode::Int3 i3Output;

		Netcode::JsonValueConverter<>::ConvertToJson(i3Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 3);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, i3Output);
		EXPECT_TRUE(i3Input.x == i3Output.x);
		EXPECT_TRUE(i3Input.y == i3Output.y);
		EXPECT_TRUE(i3Input.z == i3Output.z);
	}

	{
		Netcode::Int2 i2Input{ -8, -9 };
		Netcode::Int2 i2Output;

		Netcode::JsonValueConverter<>::ConvertToJson(i2Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 2);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, i2Output);
		EXPECT_TRUE(i2Input.x == i2Output.x);
		EXPECT_TRUE(i2Input.y == i2Output.y);
	}

	{
		Netcode::UInt4 i4Input{ 1000, 1001, 1002, 1003 };
		Netcode::UInt4 i4Output;

		Netcode::JsonValueConverter<>::ConvertToJson(i4Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 4);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, i4Output);
		EXPECT_TRUE(i4Input.x == i4Output.x);
		EXPECT_TRUE(i4Input.y == i4Output.y);
		EXPECT_TRUE(i4Input.z == i4Output.z);
		EXPECT_TRUE(i4Input.w == i4Output.w);
	}

	{
		Netcode::UInt3 i3Input{ 1004, 1005, 1006 };
		Netcode::UInt3 i3Output;

		Netcode::JsonValueConverter<>::ConvertToJson(i3Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 3);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, i3Output);
		EXPECT_TRUE(i3Input.x == i3Output.x);
		EXPECT_TRUE(i3Input.y == i3Output.y);
		EXPECT_TRUE(i3Input.z == i3Output.z);
	}

	{
		Netcode::UInt2 i2Input{ 1007, 1008 };
		Netcode::UInt2 i2Output;

		Netcode::JsonValueConverter<>::ConvertToJson(i2Input, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsArray());
		EXPECT_TRUE(v.GetArray().Size() == 2);

		Netcode::JsonValueConverter<>::ConvertFromJson(v, i2Output);
		EXPECT_TRUE(i2Input.x == i2Output.x);
		EXPECT_TRUE(i2Input.y == i2Output.y);
	}

	{
		std::wstring strInput = L"Hello World";
		std::wstring strOutput;
		
		Netcode::JsonValueConverter<>::ConvertToJson(strInput, v, doc.GetAllocator());
		EXPECT_TRUE(v.IsString());

		Netcode::JsonValueConverter<>::ConvertFromJson(v, strOutput);
		EXPECT_EQ(strInput, strOutput);
	}

	{
		Netcode::JsonValueConverter<>::ConvertToJson(Netcode::UInt2{ 1995, 2020 }, v, doc.GetAllocator());

		int32_t i;
		uint32_t ui;
		float f;
		double d;
		Netcode::Float2 f2;
		Netcode::Float3 f3;
		Netcode::Float4 f4;
		Netcode::Int2 i2;
		Netcode::Int3 i3;
		Netcode::Int4 i4;
		Netcode::UInt2 ui2;
		Netcode::UInt3 ui3;
		Netcode::UInt4 ui4;
		std::wstring str;
		
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, d), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, str), Netcode::UndefinedBehaviourException);

		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i2));
		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui2));
		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f2));

		Netcode::JsonValueConverter<>::ConvertToJson(Netcode::Float2{ 1995.0909f, 2020.0905f }, v, doc.GetAllocator());

		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, d), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, str), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i2), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui2), Netcode::UndefinedBehaviourException);
		
		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f2));

		Netcode::JsonValueConverter<>::ConvertToJson(Netcode::Float3{ 1.0f, 2.0f, 3.0f }, v, doc.GetAllocator());

		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, d), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f2), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i2), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui2), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, str), Netcode::UndefinedBehaviourException);

		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f3));

		Netcode::JsonValueConverter<>::ConvertToJson(Netcode::Float4{ 1.0f, 2.0f, 3.0f, 4.0f }, v, doc.GetAllocator());

		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, d), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f2), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i2), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui2), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, str), Netcode::UndefinedBehaviourException);
		
		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f4));

		Netcode::JsonValueConverter<>::ConvertToJson(Netcode::Int2{ -1995, -2020 }, v, doc.GetAllocator());

		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, d), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui3), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui4), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, str), Netcode::UndefinedBehaviourException);
		EXPECT_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, ui2), Netcode::UndefinedBehaviourException);

		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, i2));
		EXPECT_NO_THROW(Netcode::JsonValueConverter<>::ConvertFromJson(v, f2));
	}
}

TEST(Replication, Context) {
	Netcode::JsonDocument doc;
	doc.SetObject();
	Netcode::Network::ReplicationContext<Netcode::JsonDocument> ctx{ doc.GetAllocator(), &doc };

	int x;
	EXPECT_THROW(ctx.Get(0ull, x), Netcode::UndefinedBehaviourException);
	EXPECT_THROW(ctx.Get(L"asd", x), Netcode::OutOfRangeException);

	{ // pops on empty object
		EXPECT_TRUE(ctx.GetScopedValue() == &doc);

		ctx.Reset();
		EXPECT_TRUE(ctx.GetScopedValue() == &doc);
		ctx.Pop();
		EXPECT_TRUE(ctx.GetScopedValue() == &doc);
		
		ctx.Pop();
		ctx.Pop();
		ctx.Reset();
		ctx.Pop();
		ctx.Pop();
		EXPECT_TRUE(ctx.GetScopedValue() == &doc);
	}

	{ // push pop push reset push test
		ctx.Push(L"XYZ", rapidjson::Type::kNumberType);
		EXPECT_TRUE(ctx.GetScopedValue() != &doc);
		ctx.Set(30);
		auto * value = ctx.GetScopedValue();
		EXPECT_TRUE(value->IsNumber());
		EXPECT_TRUE(value->GetInt() == 30);

		ctx.Reset();
		EXPECT_TRUE(ctx.GetScopedValue() == &doc);
		
		ctx.Push(L"XYZ", rapidjson::Type::kNumberType);
		EXPECT_EQ(value, ctx.GetScopedValue());

		ctx.Pop();
		EXPECT_TRUE(ctx.GetScopedValue() == &doc);
		
		ctx.Push(L"XYZ", rapidjson::Type::kNumberType);
		EXPECT_EQ(value, ctx.GetScopedValue());

		EXPECT_THROW(ctx.Push(L"abc"), Netcode::UndefinedBehaviourException);
		
		ctx.Pop();
		EXPECT_TRUE(ctx.GetScopedValue() == &doc);
		EXPECT_THROW(ctx.Push(L"XYZ", rapidjson::Type::kObjectType), Netcode::UndefinedBehaviourException);
	}
	
	{
		ctx.Push(L"AAA", rapidjson::Type::kArrayType);

		auto * scopedValue = ctx.GetScopedValue();
		EXPECT_NE(scopedValue, &doc);

		ctx.Add(100);
		ctx.Add(3.0f);
		ctx.Add(200.0);

		EXPECT_EQ(scopedValue, ctx.GetScopedValue());
		EXPECT_TRUE(scopedValue->IsArray());
		EXPECT_EQ(scopedValue->GetArray().Size(), 3);
		EXPECT_TRUE((*scopedValue)[0].IsInt());
		EXPECT_TRUE((*scopedValue)[1].IsFloat());
		EXPECT_TRUE((*scopedValue)[2].IsDouble());
		
		EXPECT_EQ((*scopedValue)[0].GetInt(), 100);
		EXPECT_EQ((*scopedValue)[1].GetFloat(), 3.0f);
		EXPECT_EQ((*scopedValue)[2].GetDouble(), 200.0);

		ctx.Pop();
		EXPECT_EQ(ctx.GetScopedValue(), &doc);
	}

	{
		ctx.Push(L"BBB");
		auto * ptr1 = ctx.GetScopedValue();
		ctx.Push(L"CCC");
		auto * ptr2 = ctx.GetScopedValue();

		EXPECT_NE(ptr1, ptr2);

		ctx.Pop();
		EXPECT_EQ(ptr1, ctx.GetScopedValue());

		ctx.Push(L"CCC");
		EXPECT_EQ(ptr2, ctx.GetScopedValue());

		ctx.Set(L"DDD", Netcode::Float4::One);
		ctx.Set(L"EEE", Netcode::Float2::Zero);

		EXPECT_TRUE(ptr1->IsObject());
		EXPECT_EQ(ptr1->GetObject().MemberCount(), 1);
		EXPECT_NE(ptr1->FindMember(L"CCC"), ptr1->MemberEnd());
		
		
		EXPECT_TRUE(ptr2->IsObject());
		EXPECT_NE(ptr2->FindMember(L"DDD"), ptr2->MemberEnd());
		EXPECT_NE(ptr2->FindMember(L"EEE"), ptr2->MemberEnd());
		EXPECT_EQ(ptr2->GetObject().MemberCount(), 2);

		ctx.Reset();
		EXPECT_EQ(ctx.GetScopedValue(), &doc);

		ctx.Push(L"FFF", rapidjson::kArrayType);
		ptr1 = ctx.GetScopedValue();
		EXPECT_TRUE(ptr1->IsArray());

		ctx.Push(rapidjson::kObjectType);
		ptr2 = ctx.GetScopedValue();

		ctx.Pop();
		EXPECT_EQ(ptr1, ctx.GetScopedValue());

		ctx.Push(0ull);
		EXPECT_EQ(ptr2, ctx.GetScopedValue());
		EXPECT_TRUE(ptr2->IsObject());
		ctx.Pop();
		
		EXPECT_THROW(ctx.Push(1ull), Netcode::OutOfRangeException);
	}
}

int wmain(int argc, wchar_t * argv[]) {
	std::wstring workingDirectory = Netcode::IO::Path::CurrentWorkingDirectory();
	Netcode::IO::Path::SetWorkingDirectiory(workingDirectory);

	::testing::InitGoogleTest(&argc, argv);
	::testing::AddGlobalTestEnvironment(new Environment{ argc, argv });
	
	return RUN_ALL_TESTS();
}
