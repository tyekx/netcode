cmake_minimum_required(VERSION 3.8)

target_sources(NetcodeFoundation PUBLIC
	"Common.h"
	"Matrix.h"
	"Quaternion.h"
	"Vector2.h"
	"Vector3.h"
	"Vector4.h"
	"NonZero.hpp"

PRIVATE
	"Matrix.cpp"
	"Quaternion.cpp"
	"Vector2.cpp"
	"Vector3.cpp"
	"Vector4.cpp"
	"NonZero.cpp"
)
