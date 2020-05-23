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
	"Generated_Version.cpp"
	"Exceptions.h"
	"Math.h" "Exception/NotSupportedException.h" "Math.cpp")

add_subdirectory(Math)
