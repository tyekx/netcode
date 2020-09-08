cmake_minimum_required(VERSION 3.8)

target_sources(NetcodeClient 
PRIVATE
	Converters.h
	ClientConverters.h
	ClientConverters.cpp
	GameObjectCatalog.h
	GameObjectCatalog.cpp
)
