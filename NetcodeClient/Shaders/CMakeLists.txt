cmake_minimum_required(VERSION 3.8)

set(NETCODE_VS 
	"envMapPass_Vertex.hlsl"
	"gbufferPass_Vertex.hlsl"
	"lightingPass_Vertex.hlsl"
	"skinningPass_Vertex.hlsl"
	"sprite_Vertex.hlsl"
	"ssaoPass_Vertex.hlsl"
	"debugPrimPass_Vertex.hlsl"
)

set(NETCODE_PS
	"envmapPass_Pixel.hlsl"
	"gbufferPass_Pixel.hlsl"
	"lightingPass_Pixel.hlsl"
	"sprite_Pixel.hlsl"
	"ssaoBlurPass_Pixel.hlsl"
	"ssaoOcclusionPass_Pixel.hlsl"
	"debugPrimPass_Pixel.hlsl"
)

set(NETCODE_GS "")

set(NETCODE_HS "")

set(NETCODE_DS "")

set(NETCODE_CS 
	"skinningPassBlend_Compute.hlsl"
	"skinningPassInterpolate_Compute.hlsl" 
)


foreach(var ${NETCODE_VS})
	NETCODE_COMPILE_VS(VS_OUT ${var})
	list(APPEND NETCODE_VS_OUT ${VS_OUT})
endforeach(var)

foreach(var ${NETCODE_PS})
	NETCODE_COMPILE_PS(PS_OUT ${var})
	list(APPEND NETCODE_PS_OUT ${PS_OUT})
endforeach(var)

foreach(var ${NETCODE_GS})
	NETCODE_COMPILE_GS(GS_OUT ${var})
	list(APPEND NETCODE_GS_OUT ${PS_OUT})
endforeach(var)

foreach(var ${NETCODE_HS})
	NETCODE_COMPILE_HS(HS_OUT ${var})
	list(APPEND NETCODE_HS_OUT ${HS_OUT})
endforeach(var)

foreach(var ${NETCODE_DS})
	NETCODE_COMPILE_DS(DS_OUT ${var})
	list(APPEND NETCODE_DS_OUT ${DS_OUT})
endforeach(var)

foreach(var ${NETCODE_CS})
	NETCODE_COMPILE_CS(CS_OUT ${var})
	list(APPEND NETCODE_CS_OUT ${CS_OUT})
endforeach(var)

add_custom_target(NetcodeClientShaders DEPENDS ${NETCODE_VS_OUT} ${NETCODE_PS_OUT} ${NETCODE_GS_OUT} ${NETCODE_HS_OUT} ${NETCODE_DS_OUT} ${NETCODE_CS_OUT})
