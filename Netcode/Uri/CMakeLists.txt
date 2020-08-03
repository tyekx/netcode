cmake_minimum_required(VERSION 3.8)

target_sources(Netcode 
PUBLIC
	"Model.h"
	"Shader.h"
	"Texture.h"
	
PRIVATE
	"Model.cpp"
	"Shader.cpp"
	"Texture.cpp"
)
