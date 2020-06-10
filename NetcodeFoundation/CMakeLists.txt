cmake_minimum_required(VERSION 3.8)

add_library(NetcodeFoundation STATIC "")

target_sources(NetcodeFoundation
PUBLIC
	"ArrayView.hpp"
	"Formats.h"
	"Foundation.h"
	"MemoryFunctions.h"
	"MemoryFunctions.cpp"
	"Platform.h"
	"Sal.h"
	"Exceptions.h"
	"Math.h"
	"Math.cpp"
	"Memory.h")

add_subdirectory(Math)
add_subdirectory(Exception)
add_subdirectory(Memory)
