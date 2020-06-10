cmake_minimum_required(VERSION 3.8)

add_library(NetcodeAssetLib STATIC "")

find_package(assimp CONFIG REQUIRED)
find_package(json11 CONFIG REQUIRED)

target_include_directories(NetcodeAssetLib
PRIVATE 
	"${PROJECT_SOURCE_DIR}"
)

target_sources(NetcodeAssetLib
PUBLIC
	Animation.h
	Bone.h
	Collider.h
	Exporter.h
	Importer.h
	JsonUtility.h
	Manifest.h
	Material.h
	Mesh.h
	Model.h

PRIVATE
	Exporter.cpp
	Importer.cpp
	JsonUtility.cpp
	Manifest.cpp
	Material.cpp
	Model.cpp
)

target_link_libraries(NetcodeAssetLib
PRIVATE
	assimp::assimp
)
