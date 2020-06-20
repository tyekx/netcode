cmake_minimum_required(VERSION 3.8)

target_sources(Netcode
PUBLIC
	"File.h"
	"Directory.h"
	"Config.h"
	"Path.h"
	"BinaryReader.h"
	"BinaryWriter.h"

PRIVATE
	"File.cpp"
	"Directory.cpp"
	"Path.cpp"
	"BinaryReader.cpp"
	"BinaryWriter.cpp"
)
