cmake_minimum_required(VERSION 3.8)

add_library(Netcode STATIC "")

target_include_directories(Netcode
PUBLIC 
	"${PROJECT_SOURCE_DIR}"
)

target_include_directories(Netcode SYSTEM
PUBLIC
	"${PHYSX_INCLUDE}"
	"${BOOST_INCLUDE}"
	"${DXTEX_INCLUDE}"
	"${LUA_INCLUDE}"
	"${JSON11_INCLUDE}"
	"${SPDLOG_INCLUDE}"
	"${PROTOBUF_INCLUDE}"
	"${MYSQLCONN_INCLUDE}"
)

target_link_libraries(Netcode NetcodeFoundation)

target_sources(Netcode
PUBLIC
	AnimationBlender.cpp
	AnimationBlender.h
	AnimationState.cpp
	AnimationState.h
	BasicGeometry.cpp
	BasicGeometry.h
	BinaryReader.cpp
	BinaryReader.h
	Blackboard.cpp
	Blackboard.h
	BulkAllocator.hpp
	BulkVector.hpp
	CMakeLists.txt
	Common.cpp
	Common.h
	DebugPhysx.cpp
	DebugPhysx.h
	DebugPhysxActor.cpp
	DebugPhysxActor.h
	DebugPhysxRaycast.h
	DefaultModuleFactory.cpp
	DefaultModuleFactory.h
	DestructiveCopyConstructible.hpp
	NetcodeMath.cpp
	NetcodeMath.h
	NetcodeMpl.hpp
	Event.hpp
	FancyIterators.hpp
	File.h
	HandleTypes.h
	Input.cpp
	Input.h
	LinearAllocator.h
	LinearClassifier.cpp
	LinearClassifier.h
	Logger.cpp
	Logger.h
	Modules.cpp
	Modules.h
	ModulesConfig.h
	MovementController.h
	Path.cpp
	Path.h
	PhysXWrapper.cpp
	PhysXWrapper.h
	ProgramArgs.h
	Prototyping.h
	Service.hpp
	Shared.h
	Stopwatch.cpp
	Stopwatch.h
	Transition.cpp
	Transition.h
	Utility.cpp
	Utility.h
	Vertex.h
	WinapiWindowModule.cpp
	WinapiWindowModule.h
	ConsoleModule.h
	ConsoleModule.cpp)

add_subdirectory(Shaders)
add_subdirectory(Graphics)
add_subdirectory(Graphics/DX12)
add_subdirectory(Network)

add_dependencies(Netcode NetcodeShaders)
add_dependencies(Netcode NetcodeFoundation)
add_dependencies(Netcode NetcodeAssetLib)
