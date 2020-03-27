cmake_minimum_required(VERSION 3.8)

# target_sources now uses proper relative paths
set(CMAKE_POLICY_DEFAULT_CMP0076 NEW)

# include sdk paths
include(CMakeSDK.txt)

# c++ compiler settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-microsoft-include")

# c++ definitions
add_definitions(
	-DWIN32_LEAN_AND_MEAN
	-DNOMINMAX
	-D_CXX17_DEPRECATE_ITERATOR_BASE_CLASS
	-DBOOST_ALL_NO_LIB
)

# shader related stuff
if(NOT EXISTS "${CMAKE_BINARY_DIR}/Shaders")
	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Shaders")
endif()

function(netcode_compile_shader var_binary_path source_path shader_type entry_point)
  get_filename_component(source_filename ${source_path} NAME_WE)
  get_filename_component(source_path "${source_path}" ABSOLUTE)
  set(binary_path "${CMAKE_CURRENT_BINARY_DIR}/${source_filename}.cso")
  add_custom_command(OUTPUT ${binary_path}
    COMMENT "Generating ${binary_path}..."
    COMMAND ${FXC} /T \"${shader_type}\" /E\"${entry_point}\" /Od /Zi /Fo \"${binary_path}\" \"${source_path}\"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS "${source_path}"
  )
  set(${var_binary_path} "${binary_path}" PARENT_SCOPE)
endfunction()

macro(NETCODE_COMPILE_VS ownerTarget fileName)
	NETCODE_COMPILE_SHADER(${ownerTarget} ${fileName} "vs_5_0" "main")
endmacro()

macro(NETCODE_COMPILE_PS ownerTarget fileName)
	NETCODE_COMPILE_SHADER(${ownerTarget} ${fileName} "ps_5_0" "main")
endmacro()

macro(NETCODE_COMPILE_GS ownerTarget fileName)
	NETCODE_COMPILE_SHADER(${ownerTarget} ${fileName} "gs_5_0" "main")
endmacro()

macro(NETCODE_COMPILE_HS ownerTarget fileName)
	NETCODE_COMPILE_SHADER(${ownerTarget} ${fileName} "hs_5_0" "main")
endmacro()

macro(NETCODE_COMPILE_DS ownerTarget fileName)
	NETCODE_COMPILE_SHADER(${ownerTarget} ${fileName} "ds_5_0" "main")
endmacro()

macro(NETCODE_COMPILE_CS ownerTarget fileName)
	NETCODE_COMPILE_SHADER(${ownerTarget} ${fileName} "cs_5_0" "main")
endmacro()

project("Netcode")

add_subdirectory("NetcodeFoundation")
add_subdirectory("NetcodeLib")
add_subdirectory("NetcodeAssetLib")
add_subdirectory("Netcode")
add_subdirectory("NetcodeClient")

