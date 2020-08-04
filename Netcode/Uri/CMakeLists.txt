cmake_minimum_required(VERSION 3.8)

target_sources(Netcode 
PUBLIC
	"AssetBase.h"
	"Model.h"
	"Shader.h"
	"Texture.h"
	
PRIVATE
	"AssetBase.cpp"
	"Model.cpp"
	"Shader.cpp"
	"Texture.cpp"

)
