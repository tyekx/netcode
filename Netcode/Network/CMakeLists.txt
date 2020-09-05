cmake_minimum_required(VERSION 3.8)

target_sources(Netcode 
PUBLIC
	"GameSession.h"
	"ClientSession.h"
	"NetcodeNetworkModule.h"
	"NetworkCommon.h"
	"Cookie.h"
	"HttpSession.h"
	"ServerSession.h"
	"MysqlSession.h"
	"NetworkDecl.h"
	"Response.hpp"
	"ReplicationContext.h"
	
PRIVATE
	"GameSession.cpp"
	"ClientSession.cpp"
	"NetcodeNetworkModule.cpp"
	"NetworkCommon.cpp"
	"Cookie.cpp"
	"HttpSession.cpp"
	"ServerSession.cpp"
	"MysqlSession.cpp"
	"ReplicationContext.cpp"
)
