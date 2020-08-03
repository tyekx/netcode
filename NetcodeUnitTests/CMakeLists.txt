cmake_minimum_required(VERSION 3.17)

netcode_add_executable(NetcodeUnit WIN32 "")

target_link_options(NetcodeUnit PRIVATE "/SUBSYSTEM:CONSOLE")

find_package(GTest CONFIG REQUIRED)
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
	${PHYSX_LIB}
	${PHYSX_COMMON_LIB}
	${PHYSX_FOUNDATION_LIB}
	${PHYSX_CKINEMATIC_LIB}
	${PHYSX_EXT_LIB}
	${PHYSX_COOKING_LIB}
	${PHYSX_PVD_LIB})

find_library(DIRECTXTEX_LIBRARY DirectXTex)
find_library(JSON11_LIBRARY json11)

target_include_directories(NetcodeUnit
PRIVATE
	${PROJECT_SOURCE_DIR}
)

target_include_directories(NetcodeUnit SYSTEM
PRIVATE
	${PHYSX_INCLUDE_DIR}
	${DIRECTXTEX_INCLUDE_DIR}
)

target_sources(NetcodeUnit
PRIVATE
	"main.cpp"
 )

target_link_libraries(NetcodeUnit
PRIVATE
	NetcodeFoundation
	NetcodeAssetLib
	NetcodeProtocol
	Netcode
	assimp::assimp
	Boost::system
	Boost::date_time
	Boost::program_options
	protobuf::libprotobuf
	GTest::gtest
	${JSON11_LIBRARY}
	${PHYSX_LIBRARIES}
	${DIRECTXTEX_LIBRARY}
	${DX12_LIBRARIES}
)

include(GoogleTest)

gtest_discover_tests(NetcodeUnit WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}" VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

netcode_add_cp_command("${PROJECT_SOURCE_DIR}/NetcodeClient/config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json")
add_custom_target(NetcodeUnitConfig DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/config.json")
add_dependencies(NetcodeUnit NetcodeUnitConfig)
