cmake_minimum_required(VERSION 3.8)

add_library(NetcodeLib STATIC "")

include_directories(NetcodeLib "${BOOST_INCLUDE}" "${MYSQLCONN_INCLUDE}" "${JSON11_INCLUDE}")

target_sources(NetcodeLib PUBLIC 
	AField.cpp
	AField.h
	BReport.cpp
	BReport.h
	ClientReport.cpp
	ClientReport.h
	CMakeLists.txt
	Config.cpp
	Config.h
	Cookie.cpp
	Cookie.h
	Endpoint.cpp
	Endpoint.h
	Field.hpp
	Formatter.cpp
	Formatter.h
	GameServer.cpp
	GameServer.h
	IOEngine.cpp
	IOEngine.h
	Model.cpp
	Model.h
	ModelInterfaces.cpp
	ModelInterfaces.h
	Mysql.cpp
	Mysql.h
	netcode_database.h
	netcode_reports.h
	netcode_streams.h
	netcode_types.h
	netcode_utility.h
	NetworkStream.cpp
	NetworkStream.h
	NullableField.hpp
	ServerReport.cpp
	ServerReport.h
	Session.cpp
	Session.h
	Sleep.cpp
	Sleep.h
	StreamInterfaces.h
	StreamMessage.cpp
	StreamMessage.h
	TcpAcceptor.cpp
	TcpAcceptor.h
	TcpConnection.cpp
	TcpConnection.h
	TcpStream.cpp
	TcpStream.h
	Timer.cpp
	Timer.h
	UdpStream.cpp
	UdpStream.h
	User.cpp
	User.h
)
