cmake_minimum_required(VERSION 3.8)

target_sources(Netcode 
PUBLIC
	"GameSession.h"
	"ClientSession.h"
	"NetcodeNetworkModule.h"
	"NetworkCommon.h"
	"Cookie.h"
	"Connection.h"
	"HttpSession.h"
	"ServerSession.h"
	"MysqlSession.h"
	"NetworkDecl.h"
	"Response.hpp"
	"ReplicationContext.h"
	"NetAllocator.h"
	"Socket.hpp"
	"CompletionToken.h"
	"BasicPacket.hpp"
	
PRIVATE
	"GameSession.cpp"
	"ClientSession.cpp"
	"NetcodeNetworkModule.cpp"
	"NetworkCommon.cpp"
	"Connection.cpp"
	"Cookie.cpp"
	"HttpSession.cpp"
	"ServerSession.cpp"
	"MysqlSession.cpp"
	"ReplicationContext.cpp"
)

target_link_libraries(Netcode
PRIVATE
	iphlpapi.lib
)
