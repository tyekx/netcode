#include <string>
#include <iostream>
#include <Netcode/IO/File.h>
#include <Netcode/IO/Path.h>
#include <Netcode/IO/Directory.h>
#include <Netcode/Utility.h>
#include <boost/program_options.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <NetcodeAssetLib/Manifest.h>
#include <NetcodeAssetLib/IntermediateModel.h>
#include <NetcodeAssetLib/FBXImporter.h>
#include <NetcodeAssetLib/MaterialConverter.h>
#include <NetcodeAssetLib/AssetExporter.h>
#include <Netcode/DefaultModuleFactory.h>

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

static int CompileImage(const std::wstring & inputImage, const std::wstring & outputPath, const std::vector<std::wstring> & components, bool overwriteFlag, bool decomposeFlag) {
	if(!decomposeFlag) {
		std::cout << "Warning: decompose flag is not present, nothing to do" << std::endl;
		return 0;
	}

	if(components.empty()) {
		std::cout << "Error: decompose flag is present, but no components specified" << std::endl;
		return 1;
	}

	if(!Netcode::IO::File::Exists(inputImage)) {
		std::wcout << L"Error: image file was not found at: '" << inputImage << L"'" << std::endl;
		return 1;
	}

	Netcode::IO::File inputFile{ inputImage };

	std::wstring_view ext = inputFile.GetExtension();
	std::wstring_view fileName = inputFile.GetName();

	std::string extension = Netcode::Utility::ToNarrowString(std::wstring{ ext });

	Blob imgFileContent = SlurpFile(inputImage);

	cv::Mat srcImg{ cv::Size{ 1, static_cast<int>(imgFileContent.size) }, CV_8UC1, static_cast<void *>(imgFileContent.buffer.get()) };

	cv::Mat decodedImage = cv::imdecode(srcImg, cv::IMREAD_UNCHANGED);

	if(decodedImage.data == nullptr) {
		std::cout << "Error: failed to decode image" << std::endl;
		return 1;
	}

	std::vector<cv::Mat> destImages;

	cv::split(decodedImage, destImages);

	if(destImages.size() != components.size()) {
		std::cout << "Error: mismatching number of components. There are " <<
			destImages.size() << " channels but " << components.size() << " components specified." << std::endl;
		return 1;
	}

	std::vector<std::vector<uint8_t>> destBuffers;
	destBuffers.reserve(4);
	int numChannels = static_cast<int>(destImages.size());
	for(int i = 0; i < numChannels; i++) {
		// BGRA -> RGBA
		int idx = (i > 2) ? 3 : ((numChannels - 1) - i);

		std::vector<uint8_t> dstBuffer;
		bool success = cv::imencode('.' + extension, destImages[idx], dstBuffer);

		if(!success) {
			std::cout << "Error: failed to encode image" << std::endl;
			return 1;
		}

		destBuffers.emplace_back(std::move(dstBuffer));
	}

	int componentIndex = 0;
	for(const std::wstring & component : components) {
		std::wstring postFix = component;
		if(outputPath.back() == L'_' && component.front() == L'_') {
			postFix.assign(component.begin() + 1, component.end());
		} else if(outputPath.back() != L'_' && component.front() != L'_') {
			postFix.insert(postFix.begin(), L'_');
		}

		std::wstring outputFilePath = outputPath;
		outputFilePath.append(std::move(postFix));
		outputFilePath.append(L".");
		outputFilePath.append(ext);

		Netcode::ArrayView<uint8_t> view{ destBuffers.at(componentIndex).data(), destBuffers.at(componentIndex).size() };

		if(!overwriteFlag && Netcode::IO::File::Exists(outputFilePath)) {
			std::wcout << L"Warning: overwrite flag was not found and the destination file exists. Skipping component: " << component << std::endl;
			continue;
		}

		Netcode::IO::File outputFile{ outputFilePath };
		Netcode::IO::FileWriter<Netcode::IO::File> writer{ outputFile };
		writer->Write(view);
		writer->Close();

		componentIndex++;
	}

	return 0;
}

static int CompileAsset(const std::wstring & inputManifest, const std::wstring & outputPath, bool overwriteFlag, bool skipTransformation) {

	if(!Netcode::IO::File::Exists(inputManifest)) {
		std::wcout << L"Error: manifest file was not found at: '" << inputManifest << L"'" << std::endl;
		return 1;
	}

	if(!overwriteFlag && Netcode::IO::File::Exists(outputPath)) {
		std::wcout << L"Error: output file is present and --overwrite flag was not supplied" << std::endl;
		return 1;
	}

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
}

int wmain(int argc, wchar_t * argv[]) {

	namespace po = boost::program_options;

	std::wstring inputManifest;
	std::wstring inputImage;
	std::wstring outputPath;
	std::vector<std::wstring> testV;

	po::options_description rootDesc;

	po::options_description infoDesc("General");
	po::options_description optionsDesc("Asset Options");
	po::options_description imageDesc("Image Options");

	bool overwriteFlag;
	bool skipTransformation;
	bool decomposeFlag;

	infoDesc.add_options()
		("help", "Print help message")
		("output", po::wvalue<std::wstring>(&outputPath), "Output path")
		("overwrite", po::bool_switch(&overwriteFlag)->default_value(false), "Overwrites destination file(s) if exists")
		("verbose", po::bool_switch(&verbose)->default_value(false), "Debug mode");

	optionsDesc.add_options()
		("manifest", po::wvalue<std::wstring>(&inputManifest), "Input manifest file")
		("no_transform", po::bool_switch(&skipTransformation)->default_value(false), "Skips the mesh transformation step");

	imageDesc.add_options()
		("image", po::wvalue<std::wstring>(&inputImage), "Input image file")
		("decompose", po::bool_switch(&decomposeFlag)->default_value(false), "Enables decompose function that splits one image into multiple based on color channels")
		("component", po::wvalue<std::vector<std::wstring>>(&testV)->multitoken(), "Output components (must match the channel count)");
	
	rootDesc.add(infoDesc);
	rootDesc.add(optionsDesc);
	rootDesc.add(imageDesc);

	po::variables_map vm;

	DebugLog("Parsing input arguments");

	if(argc == 1) {
		std::cout << "Arguments must be specified" << std::endl;
		std::cout << rootDesc << std::endl;
		return 1;
	}

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

	if(inputImage.empty() && inputManifest.empty()) {
		std::cout << "Error: input file was not specified" << std::endl;
		return 1;
	}

	if(!inputImage.empty() && !inputManifest.empty()) {
		std::cout << "Error: invoke with 1 input file at a time" << std::endl;
		return 1;
	}

	if(outputPath.empty()) {
		std::cout << "Error: output file was not specified" << std::endl;
		return 1;
	}

	DebugLog("Input arguments validated");

	if(!inputManifest.empty()) {
		return CompileAsset(inputManifest, outputPath, overwriteFlag, skipTransformation);
	} else if(!inputImage.empty()) {
		return CompileImage(inputImage, outputPath, testV, overwriteFlag, decomposeFlag);
	}

	return 0;
}
