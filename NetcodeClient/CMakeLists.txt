cmake_minimum_required(VERSION 3.8)

add_executable(NetcodeClient WIN32 "")

find_package(spdlog CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS date_time program_options system)
find_package(json11 CONFIG REQUIRED)

find_path(PHYSX_INCLUDE_DIR physx/PxScene.h)
find_path(DIRECTXTEX_INCLUDE_DIR DirectXTex.h)

find_library(PHYSX_LIB PhysX_64)
find_library(PHYSX_COMMON_LIB PhysXCommon_64)
find_library(PHYSX_FOUNDATION_LIB PhysXFoundation_64)
find_library(PHYSX_CKINEMATIC_LIB PhysXCharacterKinematic_static_64)
find_library(PHYSX_EXT_LIB PhysXExtensions_static_64)
find_library(PHYSX_COOKING_LIB PhysXCooking_64)
find_library(PHYSX_PVD_LIB PhysXPvdSDK_static_64)

set(PHYSX_LIBRARIES
	"${PHYSX_LIB}"
	"${PHYSX_COMMON_LIB}"
	"${PHYSX_FOUNDATION_LIB}"
	"${PHYSX_CKINEMATIC_LIB}"
	"${PHYSX_EXT_LIB}"
	"${PHYSX_COOKING_LIB}"
	"${PHYSX_PVD_LIB}")

find_library(DIRECTXTEX_LIBRARY DirectXTex)
find_library(JSON11_LIBRARY json11)

target_include_directories(NetcodeClient 
PRIVATE
	"${PROJECT_SOURCE_DIR}"
)

target_include_directories(NetcodeClient SYSTEM
PRIVATE
	"${PHYSX_INCLUDE_DIR}"
	"${DIRECTXTEX_INCLUDE_DIR}"
)

target_sources(NetcodeClient
PRIVATE
	Asset.h
	AssetManager.h
	Components.cpp
	Components.h
	ComponentStorage.hpp
	ConstantBufferTypes.h
	DevCameraScript.cpp
	DevCameraScript.h
	NetcodeApp.h
	GameObject.cpp
	GameObject.h
	GameScene.cpp
	GameScene.h
	GraphicsEngine.h
	Layer.h
	Light.h
	main.cpp
	Material.h
	Mesh.h
	PhysxHelpers.cpp
	PhysxHelpers.h
	PlayerBehavior.h
	RemoteAvatarScript.cpp
	RemoteAvatarScript.h
	Scene.cpp
	Scene.h
	Services.cpp
	Services.h
	Snippets.h
	Systems.cpp
	Systems.h
	UIButtonPrefab.h
	UIControl.h
	UILabel.h
	UIObject.h
	UIPagePrefab.h
	UIScene.h
	UIServerRow.h
	UISpinner.h
	UITextBox.h
	AnimationSet.h
	AnimationSet.cpp
	AnimationController.h
	AnimationController.cpp
)

target_link_libraries(NetcodeClient
PRIVATE 
	NetcodeFoundation
	NetcodeAssetLib
	Netcode
	assimp::assimp
	Boost::system
	Boost::date_time
	Boost::program_options
	protobuf::libprotobuf
	"${JSON11_LIBRARY}"
	"${PHYSX_LIBRARIES}"
	"${DIRECTXTEX_LIBRARY}"
	"${DX12_LIBRARIES}")

add_subdirectory(Shaders)

add_dependencies(NetcodeClient NetcodeClientShaders)
