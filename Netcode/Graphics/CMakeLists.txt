cmake_minimum_required(VERSION 3.8)

target_sources(Netcode
PUBLIC
	"FrameGraph.h"
	"GraphicsContexts.h"
	"ResourceDesc.h"
	"ResourceEnums.h"
	"UploadBatch.h"
	"Config.h"

PRIVATE
	"FrameGraph.cpp"
	"ResourceEnums.cpp"
	"UploadBatch.cpp"
)

add_subdirectory(DX12)
