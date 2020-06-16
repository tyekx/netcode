cmake_minimum_required(VERSION 3.8)

target_sources(Netcode 
PRIVATE
	"GameSession.h"
	"ClientSession.h"
	"NetcodeNetworkModule.h"
	"NetworkCommon.h"
	"Cookie.h"
	"HttpSession.h"
	"ServerSession.h"
	"MysqlSession.h"

PUBLIC
	"GameSession.cpp"
	"ClientSession.cpp"
	"NetcodeNetworkModule.cpp"
	"NetworkCommon.cpp"
	"Cookie.cpp"
	"HttpSession.cpp"
	"ServerSession.cpp"
	"MysqlSession.cpp"
)
