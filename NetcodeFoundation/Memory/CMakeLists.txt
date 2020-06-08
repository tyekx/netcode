cmake_minimum_required(VERSION 3.8)

target_sources(NetcodeFoundation PUBLIC
	"Common.h"
	"Common.cpp"
	"AlignedMallocator.hpp"
	"ResourceAllocator.hpp"
	"SpyAllocator.hpp"
	"PoolAllocator.hpp"
	"ObjectAllocator.hpp"
	"StdAllocatorAdapter.hpp"
)
