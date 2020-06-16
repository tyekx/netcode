cmake_minimum_required(VERSION 3.8)

target_sources(Netcode
PUBLIC
	"Config.h"
	"Win32Window.h"

PRIVATE
	"Win32Window.cpp"
)
