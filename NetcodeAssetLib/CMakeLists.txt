cmake_minimum_required(VERSION 3.8)

add_library(NetcodeAssetLib STATIC "")

target_include_directories(NetcodeAssetLib PUBLIC 
	"${PROJECT_SOURCE_DIR}"
)

target_include_directories(NetcodeAssetLib SYSTEM
PUBLIC
	"${JSON11_INCLUDE}"
	"${ASSIMP_INCLUDE}"
	"${DXTEX_INCLUDE}"
)

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
