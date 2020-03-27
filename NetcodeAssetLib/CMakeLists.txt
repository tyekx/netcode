cmake_minimum_required(VERSION 3.8)

add_library(NetcodeAssetLib STATIC "")

include_directories(NetcodeAssetLib "${PROJECT_SOURCE_DIR}")
include_directories(NetcodeAssetLib "${JSON11_INCLUDE}")
include_directories(NetcodeAssetLib "${ASSIMP_INCLUDE}")
include_directories(NetcodeAssetLib "${DXTEX_INCLUDE}")

target_sources(NetcodeAssetLib
PUBLIC
	Animation.h
	Bone.h
	Collider.h
	Exporter.cpp
	Exporter.h
	Importer.cpp
	Importer.h
	JsonUtility.cpp
	JsonUtility.h
	Manifest.cpp
	Manifest.h
	Material.cpp
	Material.h
	Mesh.h
	Model.cpp
	Model.h
)
