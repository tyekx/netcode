cmake_minimum_required(VERSION 3.8)

target_sources(Netcode
PUBLIC
	"FrameGraph.h"
	"GraphicsContexts.h"
	"ResourceDesc.h"
	"ResourceEnums.h"
	"UploadBatch.h"
	"SpriteDesc.h"
	"BorderDesc.h"
	"Material.h"

PRIVATE
	"ResourceEnums.cpp"
	"SpriteDesc.cpp"
	"BorderDesc.cpp"
	"Material.cpp"
)

add_subdirectory(DX12)
