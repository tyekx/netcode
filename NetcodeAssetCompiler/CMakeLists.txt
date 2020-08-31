cmake_minimum_required(VERSION 3.8)

find_package(Boost REQUIRED COMPONENTS date_time program_options system)
find_package(json11 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

find_package(OpenCV CONFIG REQUIRED)
find_library(JSON11_LIBRARY json11)

netcode_add_executable(NetcodeAssetCompiler "")


target_include_directories(NetcodeAssetCompiler 
PRIVATE
	${PROJECT_SOURCE_DIR}
)

target_sources(NetcodeAssetCompiler PRIVATE main.cpp)

target_link_libraries(NetcodeAssetCompiler PRIVATE
	Boost::program_options
	assimp::assimp
	Netcode
	NetcodeAssetLib
	${JSON11_LIBRARY}
	opencv_core
	opencv_imgcodecs
)
