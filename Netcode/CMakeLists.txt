cmake_minimum_required(VERSION 3.8)

add_library(Netcode STATIC "")

include_directories(Netcode
	"${PROJECT_SOURCE_DIR}"
	"${PHYSX_INCLUDE}"
	"${BOOST_INCLUDE}"
	"${DXTEX_INCLUDE}"
	"${LUA_INCLUDE}"
	"${JSON11_INCLUDE}"
	"${SPDLOG_INCLUDE}"
)

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
	HandleTypes.cpp
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
	NetcodeNetworkModule.cpp
	NetcodeNetworkModule.h
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
)

add_subdirectory(Shaders)
add_subdirectory(Graphics)
add_subdirectory(Graphics/DX12)

add_dependencies(Netcode NetcodeShaders)
