cmake_minimum_required(VERSION 3.8)

target_sources(Netcode PUBLIC
	Blender.h
	Blender.cpp
	Blackboard.h
	Blackboard.cpp
	State.h
	State.cpp
	StateGroup.h
	StateGroup.cpp
	Transition.h
	Transition.cpp
	IK.h
	IK.cpp
)
