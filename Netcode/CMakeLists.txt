cmake_minimum_required(VERSION 3.8)

add_library(Netcode STATIC "")

find_package(sqlite3 CONFIG REQUIRED)
find_package(json11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS date_time program_options system)

find_path(PHYSX_INCLUDE_DIR physx/PxScene.h)

find_path(DIRECTXTEX_INCLUDE_DIR DirectXTex.h)
find_library(DIRECTXTEX_LIBRARY DirectXTex)

target_include_directories(Netcode
PRIVATE 
	"${PROJECT_SOURCE_DIR}"
)

target_include_directories(Netcode SYSTEM
PRIVATE
	"${PHYSX_INCLUDE_DIR}"
)

target_link_libraries(Netcode
PRIVATE
	NetcodeFoundation
	spdlog::spdlog
	Boost::program_options
	Boost::system
	Boost::date_time
	sqlite3
	"${DXTEX_LINK}"
)

target_sources(Netcode
PUBLIC
	BasicGeometry.cpp
	BasicGeometry.h
	BinaryReader.cpp
	BinaryReader.h
	BulkAllocator.hpp
	BulkVector.hpp
	CMakeLists.txt
	Common.cpp
	Common.h
	DebugPhysx.cpp
	DebugPhysx.h
	DebugPhysxActor.cpp
	DebugPhysxActor.h
	DebugPhysxRaycast.h
	DefaultModuleFactory.cpp
	DefaultModuleFactory.h
	DestructiveCopyConstructible.hpp
	NetcodeMath.cpp
	NetcodeMath.h
	NetcodeMpl.hpp
	Event.hpp
	FancyIterators.hpp
	File.h
	HandleTypes.h
	Input.cpp
	Input.h
	LinearAllocator.h
	LinearClassifier.cpp
	LinearClassifier.h
	Logger.cpp
	Logger.h
	Modules.cpp
	Modules.h
	ModulesConfig.h
	MovementController.h
	Path.cpp
	Path.h
	PhysXWrapper.cpp
	PhysXWrapper.h
	ProgramArgs.h
	Service.hpp
	Shared.h
	Stopwatch.cpp
	Stopwatch.h
	Utility.cpp
	Utility.h
	Vertex.h
	WinapiWindowModule.cpp
	WinapiWindowModule.h
	ConsoleModule.h
	ConsoleModule.cpp
	"Functions.h"
	"Functions.cpp"
	"GramSchmidt.h"
	"GramSchmidt.cpp")

add_subdirectory(Shaders)
add_subdirectory(Animation)
add_subdirectory(Graphics)
add_subdirectory(Graphics/DX12)
add_subdirectory(Network)

add_dependencies(Netcode NetcodeShaders)
add_dependencies(Netcode NetcodeFoundation)
add_dependencies(Netcode NetcodeAssetLib)
