cmake_minimum_required(VERSION 3.8)

target_sources(NetcodeClient 
PRIVATE
	ReplDesc.h
	Replicator.hpp
	ReplArguments.hpp
	GameClient.h
	GameClient.cpp
	GameServer.h
	GameServer.cpp
	NetwUtil.h
	NetwUtil.cpp
)
