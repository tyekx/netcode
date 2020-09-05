cmake_minimum_required(VERSION 3.8)

netcode_add_executable(NetcodeServer "")

find_package(json11 CONFIG REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS date_time program_options system)
find_package(gRPC CONFIG REQUIRED)

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

target_sources(NetcodeServer
PRIVATE
	"main.cpp"
	"ServerApp.h"
	"ServerApp.cpp"
	"ProgramOptions.cpp"
	"ProgramOptions.h"
 "RpcService.h" "RpcService.cpp")

target_include_directories(NetcodeServer PRIVATE
	${PROJECT_SOURCE_DIR}
)

target_include_directories(NetcodeServer
PRIVATE SYSTEM
	${MYSQLCONN_INCLUDE}
	${DIRECTXTEX_INCLUDE_DIR}
)

target_link_directories(NetcodeServer
PRIVATE
	${MYSQLCONN_LIB}
)

# Note: 7 target(s) were omitted.
# target_link_libraries(main PRIVATE gRPC::gpr gRPC::grpc gRPC::grpc++ gRPC::grpc++_alts)

# find_package(modules CONFIG REQUIRED)
# target_link_libraries(main PRIVATE c-ares::cares)
	
target_link_libraries(NetcodeServer
PRIVATE
	Netcode
	NetcodeProtocol
	protobuf::libprotobuf
	Boost::date_time
	Boost::system
	Boost::program_options
	gRPC::grpc++
	${JSON11_LIBRARY}
	${DIRECTXTEX_LIBRARY}
	${PHYSX_LIBRARIES}
	${MYSQLCONN_LINK}
	${DX12_LIBRARIES}
)

netcode_add_cp_command("config.json" "${CMAKE_CURRENT_BINARY_DIR}/config.json")

add_custom_target(NetcodeServerConfig DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/config.json")

add_dependencies(NetcodeServer NetcodeServerConfig)
