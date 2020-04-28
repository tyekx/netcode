cmake_minimum_required(VERSION 3.8)

add_library(NetcodeFoundation STATIC
	"ArrayView.hpp"
	"Formats.h"
	"Foundation.h"
	"MemoryFunctions.h"
	"MemoryFunctions.cpp"
	"Platform.h"
	"Sal.h"
	"Generated_Version.cpp")
