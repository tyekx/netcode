cmake_minimum_required(VERSION 3.8)

target_sources(NetcodeFoundation
PUBLIC
	"Common.h"
	"ArithmeticException.h"
	"IOException.h"
	"NotSupportedException.h"
	"OutOfRangeException.h"
	"UndefinedBehaviourException.h"
	"RuntimeException.h"

PRIVATE
	"Exception.cpp"	
)
