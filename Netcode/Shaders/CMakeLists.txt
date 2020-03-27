cmake_minimum_required(VERSION 3.8)

set(NETCODE_VS "")

set(NETCODE_PS "")

set(NETCODE_GS "")

set(NETCODE_HS "")

set(NETCODE_DS "")

set(NETCODE_CS "")

find_program(FXC fxc)

if(NOT FXC) 
	message(FATAL_ERROR "Shader compiler was not found")
endif()

add_custom_target(NetcodeShaders SOURCES ${NETCODE_VS} ${NETCODE_PS} ${NETCODE_GS} ${NETCODE_HS} ${NETCODE_DS} ${NETCODE_CS})

foreach(var ${NETCODE_VS})
	NETCODE_COMPILE_VS(NetcodeShaders ${var})
endforeach(var)

foreach(var ${NETCODE_PS})
	NETCODE_COMPILE_PS(NetcodeShaders ${var})
endforeach(var)

foreach(var ${NETCODE_GS})
	NETCODE_COMPILE_GS(NetcodeShaders ${var})
endforeach(var)

foreach(var ${NETCODE_HS})
	NETCODE_COMPILE_HS(NetcodeShaders ${var})
endforeach(var)

foreach(var ${NETCODE_DS})
	NETCODE_COMPILE_DS(NetcodeShaders ${var})
endforeach(var)

foreach(var ${NETCODE_CS})
	NETCODE_COMPILE_CS(NetcodeShaders ${var})
endforeach(var)
