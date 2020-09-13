cmake_minimum_required(VERSION 3.8)

netcode_add_static_library(NetcodeFoundation)

find_package(RapidJSON CONFIG REQUIRED)

target_include_directories(NetcodeFoundation 
PRIVATE SYSTEM
	${RAPIDJSON_INCLUDE_DIRS}
)

target_sources(NetcodeFoundation
PUBLIC
	"ArrayView.hpp"
	"Formats.h"
	"Foundation.h"
	"UriBase.h"
	"Platform.h"
	"Sal.h"
	"Exceptions.h"
	"Math.h"
	"Memory.h"
	"Json.h"
	"Macros.h"
	"Enum.hpp"
	
PRIVATE
	
	"Math.cpp"
	"Exceptions.cpp"
	"UriBase.cpp"
	"Json.cpp"
)

add_subdirectory(Math)
add_subdirectory(Exception)
add_subdirectory(Memory)
