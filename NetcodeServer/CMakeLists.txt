cmake_minimum_required(VERSION 3.8)

add_executable(NetcodeServer "")

find_package(protobuf CONFIG REQUIRED)

target_sources(NetcodeServer
PRIVATE
	main.cpp
	ServerApp.h
	ProgramOptions.cpp
	ProgramOptions.h
)

target_include_directories(NetcodeServer PRIVATE
	"${CMAKE_PROJECT_DIR}"
)

target_include_directories(NetcodeServer SYSTEM
PRIVATE 
	"${BOOST_INCLUDE}"
)

target_link_directories(NetcodeServer
PUBLIC
	"${ASSIMP_LIB}"
	"${PHYSX_LIB}"
	"${JSON11_LIB}"
	"${DXTEX_LIB}"
	"${BOOST_LIB}"
	"${MYSQLCONN_LIB}"
)

target_link_libraries(NetcodeServer Netcode NetcodeProtocol protobuf::libprotobuf
	"${ASSIMP_LINK}"
	"${DXTEX_LINK}"
	"${BOOST_LINK}"
	"${MYSQLCONN_LINK}"
	"${PHYSX_LINK}"
	"${JSON11_LINK}"
	"${DX12_LINK}"
)

netcode_add_cp_command("config.cfg" "${CMAKE_CURRENT_BINARY_DIR}/config.cfg")

add_custom_target(NetcodeServerConfig DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/config.cfg")

add_dependencies(NetcodeServer NetcodeServerConfig)

netcode_increment_build_number(NetcodeServer)
