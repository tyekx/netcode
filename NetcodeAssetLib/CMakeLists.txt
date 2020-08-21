cmake_minimum_required(VERSION 3.8)

netcode_add_static_library(NetcodeAssetLib)

find_package(assimp CONFIG REQUIRED)
find_package(RapidJSON CONFIG REQUIRED)

target_include_directories(NetcodeAssetLib
PRIVATE 
	${PROJECT_SOURCE_DIR}
)

target_sources(NetcodeAssetLib
PUBLIC
	"Animation.h"
	"Bone.h"
	"Collider.h"
	"Exporter.h"
	"Importer.h"
	"JsonUtility.h"
	"Manifest.h"
	"Material.h"
	"Mesh.h"
	"Model.h"
	"AssetExporter.h"
	"BoundingBoxHelpers.h"
	"FBXImporter.h"
	"IntermediateModel.h"
	"MaterialConverter.h"

PRIVATE
	"Exporter.cpp"
	"Importer.cpp"
	"JsonUtility.cpp"
	"Manifest.cpp"
	"Material.cpp"
	"Model.cpp"
	"AssetExporter.cpp"
	"BoundingBoxHelpers.cpp"
	"FBXImporter.cpp"
	"IntermediateModel.cpp"
	"MaterialConverter.cpp"
)

target_include_directories(NetcodeAssetLib SYSTEM
PRIVATE
	${RAPIDJSON_INCLUDE_DIRS}
)

target_link_libraries(NetcodeAssetLib
PRIVATE
	assimp::assimp
)
