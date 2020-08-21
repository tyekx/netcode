#include <string>
#include <iostream>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Path.h>
#include <Netcode/Utility.h>
#include <boost/program_options.hpp>
#include <NetcodeAssetLib/Manifest.h>
#include <NetcodeAssetLib/IntermediateModel.h>
#include <NetcodeAssetLib/FBXImporter.h>
#include <NetcodeAssetLib/MaterialConverter.h>
#include <NetcodeAssetLib/AssetExporter.h>

struct Blob {
	std::unique_ptr<uint8_t[]> buffer;
	size_t size;
};

Blob SlurpFile(const std::wstring & path) {
	Netcode::IO::File file{ path };
	Netcode::IO::FileReader<Netcode::IO::File> reader{ file, Netcode::IO::FileOpenMode::READ };
	size_t size = reader->GetSize();
	std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(size);
	Netcode::MutableArrayView<uint8_t> mutableView{ buffer.get(), size };
	reader->Read(mutableView);
	reader->Close();

	Blob b;
	b.buffer = std::move(buffer);
	b.size = size;
	return b;
}

static bool verbose = false;

static void DebugLog(const char * str) {
	if(verbose) {
		std::cout << str << std::endl;
	}
}

int wmain(int argc, wchar_t * argv[]) {

	namespace po = boost::program_options;

	std::wstring inputManifest;
	std::wstring outputPath;

	po::options_description rootDesc;

	po::options_description infoDesc("Information");
	po::options_description optionsDesc("Options");

	bool overwriteFlag;
	bool skipTransformation;

	infoDesc.add_options()
		("help", "Print help message");

	optionsDesc.add_options()
		("manifest", po::wvalue<std::wstring>(&inputManifest), "Input manifest file")
		("output", po::wvalue<std::wstring>(&outputPath), "Output path")
		("overwrite", po::bool_switch(&overwriteFlag)->default_value(false), "Overwrites destination file if exists")
		("no_transform", po::bool_switch(&skipTransformation)->default_value(false), "Skips the mesh transformation step")
		("verbose", po::bool_switch(&verbose)->default_value(false), "Debug mode");
	
	rootDesc.add(infoDesc);
	rootDesc.add(optionsDesc);

	po::variables_map vm;

	DebugLog("Parsing input arguments");

	try {
		po::store(po::wcommand_line_parser(argc, argv).options(rootDesc).run(), vm);
		po::notify(vm);
	} catch(po::error & e) {
		std::cout << "Error: " << e.what() << std::endl;
		std::cout << rootDesc << std::endl;
		return 1;
	}

	if(vm.find("help") != vm.end()) {
		std::cout << rootDesc << std::endl;
		return 0;
	}

	if(inputManifest.empty()) {
		std::cout << "Error: manifest file is required" << std::endl;
		return 1;
	}

	if(outputPath.empty()) {
		std::cout << "Error: output file was not specified" << std::endl;
		return 1;
	}

	if(!Netcode::IO::File::Exists(inputManifest)) {
		std::wcout << L"Error: manifest file was not found at: '" << inputManifest << L"'" << std::endl;
		return 1;
	}

	if(!overwriteFlag && Netcode::IO::File::Exists(outputPath)) {
		std::wcout << L"Error: output file is present and --overwrite flag was not supplied" << std::endl;
		return 1;
	}

	DebugLog("Input arguments validated");

	Netcode::IO::Path::SetWorkingDirectiory(Netcode::IO::Path::CurrentWorkingDirectory());


	DebugLog("Loading manifest file");
	Blob jsonFile = SlurpFile(inputManifest);
	std::string content{ std::string_view { reinterpret_cast<char *>(jsonFile.buffer.get()), jsonFile.size } };

	std::string err;
	json11::Json manifestJson = json11::Json::parse(content, err);

	if(!err.empty()) {
		std::cout << "Error: failed to parse json, " << err << std::endl;
		return 1;
	}

	Netcode::Asset::Manifest manifest;
	manifest.Load(manifestJson);

	DebugLog("Manifest loaded, loading FBX content");

	Netcode::Intermediate::Model model;

	{
		Blob baseFbx = SlurpFile(Netcode::Utility::ToWideString(manifest.base.file));
		model = Netcode::FBXImporter::FromMemory(baseFbx.buffer.get(), baseFbx.size);
	}
	
	DebugLog("Base FBX loaded, loading referenced FBX files");

	for(const auto & animRef : manifest.animations) {
		Blob fbxContent = SlurpFile(Netcode::Utility::ToWideString(animRef.source.file));
		auto animation = Netcode::FBXImporter::ImportAnimationsFromMemory(fbxContent.buffer.get(), fbxContent.size, model.skeleton);
		auto optimizedAnim = Netcode::FBXImporter::OptimizeAnimation(animation.front(), model.skeleton);
		optimizedAnim.name = animRef.name;
		model.animations.emplace_back(std::move(optimizedAnim));
	}

	DebugLog("All FBX files have been processed");

	if(!manifest.materials.empty()) {
		model.materials = Netcode::ConvertToNetcodeMaterials(manifest.materials);
		DebugLog("Materials imported from manifest, overriding FBX materials");
	}

	model.colliders = manifest.colliders;
	model.offlineTransform = manifest.offlineTransform;

	if(!skipTransformation) {

		for(Netcode::Intermediate::Mesh & m : model.meshes) {
			m.ApplyTransformation(model.offlineTransform);
		}
		DebugLog("Transformation applied");

	} else {
		DebugLog("Transformation phase skipped");
	}


	DebugLog("Exporting into ncasset format");
	auto [rawData, rawDataSize] = Netcode::AssetExporter::Export(model);

	DebugLog("Writing ncasset");
	Netcode::IO::File outputFile{ outputPath };
	Netcode::IO::FileWriter<Netcode::IO::File> writer{ outputFile };
	writer->Write(Netcode::ArrayView<uint8_t>(rawData.get(), rawDataSize));
	writer->Close();

	std::wcout << L"Compilation successful. Written file: '" << outputPath << L"', size: " << rawDataSize << std::endl;

	return 0;
}
