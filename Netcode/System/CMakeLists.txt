cmake_minimum_required(VERSION 3.8)

target_sources(Netcode
PUBLIC
	"SystemClock.h"
	"TimeTypes.h"
	"FpsCounter.h"
	"System.h"
	"SecureString.h"
	"GameClock.h"
PRIVATE
	"SystemClock.cpp"
	"FpsCounter.cpp"
	"System.cpp"
	"SecureString.cpp"
	"GameClock.cpp"
)
