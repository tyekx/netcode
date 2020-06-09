cmake_minimum_required(VERSION 3.8)

add_executable(NetcodeClient WIN32 "")

find_package(spdlog CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)


target_include_directories(NetcodeClient 
PRIVATE
	"${PROJECT_SOURCE_DIR}"
)

target_include_directories(NetcodeClient SYSTEM
PRIVATE
	"${MYSQLCONN_INCLUDE}"
	"${BOOST_INCLUDE}"
	"${JSON11_INCLUDE}"
	"${PHYSX_INCLUDE}"
	"${DXTEX_INCLUDE}"
)

target_sources(NetcodeClient
PUBLIC
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

target_link_directories(NetcodeClient
PUBLIC
	"${PHYSX_LIB}"
	"${JSON11_LIB}"
	"${DXTEX_LIB}"
	"${BOOST_LIB}"
	"${MYSQLCONN_LIB}"
)

target_link_libraries(NetcodeClient PRIVATE assimp::assimp protobuf::libprotobuf
	NetcodeFoundation
	NetcodeAssetLib
	Netcode
	"${DXTEX_LINK}"
	"${BOOST_LINK}"
	"${MYSQLCONN_LINK}"
	"${PHYSX_LINK}"
	"${JSON11_LINK}"
	"${DX12_LINK}")

add_subdirectory(Shaders)

add_dependencies(NetcodeClient NetcodeClientShaders)
