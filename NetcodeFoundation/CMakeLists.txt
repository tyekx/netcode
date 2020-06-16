cmake_minimum_required(VERSION 3.8)

netcode_add_static_library(NetcodeFoundation)

target_sources(NetcodeFoundation
PUBLIC
	"ArrayView.hpp"
	"Formats.h"
	"Foundation.h"
	"MemoryFunctions.h"
	"Platform.h"
	"Sal.h"
	"Exceptions.h"
	"Math.h"
	"Memory.h"
	
PRIVATE
	"MemoryFunctions.cpp"
	"Math.cpp"
)

add_subdirectory(Math)
add_subdirectory(Exception)
add_subdirectory(Memory)
