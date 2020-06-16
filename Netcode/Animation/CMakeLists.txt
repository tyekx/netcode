cmake_minimum_required(VERSION 3.8)

target_sources(Netcode
PUBLIC
	"Blender.h"
	"Blackboard.h"
	"State.h"
	"StateGroup.h"
	"Transition.h"
	"IK.h"

PRIVATE
	"Blender.cpp"
	"Blackboard.cpp"
	"State.cpp"
	"StateGroup.cpp"
	"Transition.cpp"
	"IK.cpp"
)
