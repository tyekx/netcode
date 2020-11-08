cmake_minimum_required(VERSION 3.8)

netcode_add_static_library(Netcode)

find_package(RapidJSON CONFIG REQUIRED)
find_package(json11 CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS date_time program_options system)
find_package(OpenSSL REQUIRED)

find_path(PHYSX_INCLUDE_DIR physx/PxScene.h)

find_path(DIRECTXTEX_INCLUDE_DIR DirectXTex.h)
find_library(DIRECTXTEX_LIBRARY DirectXTex)

find_path(MYSQL_INCLUDE_DIR mysqlx/xdevapi.h)
find_library(MYSQL_LIBRARY mysqlcppconn8)

target_include_directories(Netcode
PRIVATE 
	${PROJECT_SOURCE_DIR}
)

target_include_directories(Netcode 
PRIVATE SYSTEM
	${PHYSX_INCLUDE_DIR}
	${MYSQL_INCLUDE_DIR}
	${RAPIDJSON_INCLUDE_DIRS}
)

target_link_libraries(Netcode
PRIVATE
	NetcodeProtocol
	NetcodeFoundation
	spdlog::spdlog
	Boost::program_options
	Boost::system
	Boost::date_time
	OpenSSL::SSL OpenSSL::Crypto
	${MYSQL_LIBRARY}
	${DIRECTXTEX_LIBRARY}
)

target_sources(Netcode
PUBLIC
	"BasicGeometry.h"
	"Common.h"
	"DebugPhysx.h"
	"DebugPhysxActor.h"
	"DebugPhysxRaycast.h"
	"DefaultModuleFactory.h"
	"MathExt.h"
	"HandleTypes.h"
	"Input.h"
	"LinearAllocator.h"
	"LinearClassifier.h"
	"Logger.h"
	"Modules.h"
	"ModulesConfig.h"
	"MovementController.h"
	"PhysXWrapper.h"
	"BulkVector.hpp"
	"DestructiveCopyConstructible.hpp"
	"NetcodeMpl.hpp"
	"Event.hpp"
	"FancyIterators.hpp"
	"Service.hpp"
	"Stopwatch.h"
	"Utility.h"
	"Vertex.h"
	"WinapiWindowModule.h"
	"ConsoleModule.h"
	"Config.h"
	"HandleDecl.h"
	"PxPtr.hpp"

PRIVATE
	"BasicGeometry.cpp"
	"BulkAllocator.hpp"
	"Common.cpp"
	"DebugPhysx.cpp"
	"DebugPhysxActor.cpp"
	"DefaultModuleFactory.cpp"
	"MathExt.cpp"
	"Input.cpp"
	"LinearClassifier.cpp"
	"Logger.cpp"
	"Modules.cpp"
	"PhysXWrapper.cpp"
	"Stopwatch.cpp"
	"Utility.cpp"
	"WinapiWindowModule.cpp"
	"ConsoleModule.cpp"
	"Functions.cpp"
	"Config.cpp")

add_subdirectory(Shaders)
add_subdirectory(Animation)
add_subdirectory(Graphics)
add_subdirectory(Network)
add_subdirectory(Window)
add_subdirectory(IO)
add_subdirectory(Sync)
add_subdirectory(System)
add_subdirectory(UI)
add_subdirectory(URI)

add_dependencies(Netcode NetcodeShaders NetcodeFoundation NetcodeAssetLib NetcodeProtocol)
