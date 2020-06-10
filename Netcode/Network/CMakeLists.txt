cmake_minimum_required(VERSION 3.8)

target_sources(Netcode 
PUBLIC
	GameSession.cpp
	GameSession.h
	ClientSession.h
	ClientSession.cpp
	NetcodeNetworkModule.h
	NetcodeNetworkModule.cpp
	NetworkCommon.h
	NetworkCommon.cpp
	Cookie.h
	Cookie.cpp
	HttpSession.h
	HttpSession.cpp
	ServerSession.h
	ServerSession.cpp
	MysqlSession.h
	MysqlSession.cpp
)

target_link_libraries(Netcode PRIVATE NetcodeProtocol)
