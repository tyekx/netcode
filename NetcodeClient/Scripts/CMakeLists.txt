cmake_minimum_required(VERSION 3.8)

target_sources(NetcodeClient 
PRIVATE
	DebugScript.h
	DebugScript.cpp
	DevCameraScript.h
	DevCameraScript.cpp
	GunScript.h
	GunScript.cpp
	LocalPlayerScript.h
	LocalPlayerScript.cpp
	RemotePlayerScript.h
	RemotePlayerScript.cpp
)
