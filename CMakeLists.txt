cmake_minimum_required(VERSION 3.8)

project("Netcode" VERSION 0.0.1 HOMEPAGE_URL "https://netcode-game.eu" LANGUAGES CXX)

# target_sources now uses proper relative paths
set(CMAKE_POLICY_DEFAULT_CMP0076 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0079 NEW)

set(DX12_LIBRARIES "dxgi.lib d3d12.lib d3dcompiler.lib dwrite.lib d2d1.lib d3d11.lib")

set(NETCODE_COMPILE_OPTIONS
	"-Wno-microsoft-include"
	"-Wno-unused-private-field"
	"-Wno-unknown-pragmas"
	"-Wno-unknown-warning-option"
	"-Wno-return-std-move"
	"/EHa")

set(NETCODE_COMPILE_DEFINITIONS 
	"-DWIN32_LEAN_AND_MEAN"
	"-DNOMINMAX"
	"-D_CXX17_DEPRECATE_ITERATOR_BASE_CLASS"
	"-DBOOST_BIND_NO_PLACEHOLDERS"
	"-D_WIN32_WINNT=0x0A00"
	"-D_CRT_SECURE_NO_WARNINGS"
	"-DRAPIDJSON_HAS_STDSTRING=1")

function(NETCODE_ADD_COMPILE_DEFINITION def)
	list(APPEND NETCODE_COMPILE_DEFINITIONS ${def})
endfunction()

if(DEFINED NETCODE_NO_WINAPI)
	netcode_add_compile_definition("-DNETCODE_NO_WINAPI")
endif()

macro(NETCODE_ADD_EXECUTABLE target_name args)
	add_executable(${target_name} ${args})
	target_compile_options(${target_name} PRIVATE ${NETCODE_COMPILE_OPTIONS})
	target_compile_features(${target_name} PRIVATE cxx_std_17)
	target_compile_definitions(${target_name} PRIVATE ${NETCODE_COMPILE_DEFINITIONS})
endmacro()

macro(NETCODE_ADD_STATIC_LIBRARY target_name)
	add_library(${target_name} STATIC "")
	target_compile_options(${target_name} PRIVATE ${NETCODE_COMPILE_OPTIONS})
	target_compile_features(${target_name} PRIVATE cxx_std_17)
	target_compile_definitions(${target_name} PRIVATE ${NETCODE_COMPILE_DEFINITIONS})
endmacro()

macro(NETCODE_ADD_SHARED_LIBRARY target_name)
	add_library(${target_name} SHARED "")
	target_compile_options(${target_name} PRIVATE ${NETCODE_COMPILE_OPTIONS})
	target_compile_features(${target_name} PRIVATE cxx_std_17)
	target_compile_definitions(${target_name} PRIVATE ${NETCODE_COMPILE_DEFINITIONS})
endmacro()

# shader related stuff
if(NOT EXISTS "${CMAKE_BINARY_DIR}/Shaders")
	file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/Shaders")
endif()

find_program(FXC fxc)

if(NOT FXC) 
	message(FATAL_ERROR "Shader compiler was not found")
endif()

find_program(PROTOC protoc PATHS "C:\\work\\vcpkg\\packages\\protobuf_x64-windows\\tools\\protobuf")

if(NOT PROTOC)
	message(FATAL_ERROR "Protobuf compiler was not found")
endif()

function(netcode_compile_proto var_generated_headers var_generated_source source_path_list)
	set(generated_header_files "")
	set(generated_source_files "")
	set(abs_path_generated_files "")
	set(abs_path_proto_files "")
	foreach(proto_file ${source_path_list})
		get_filename_component(source_filename ${proto_file} NAME_WE)
		get_filename_component(source_path "${proto_file}" ABSOLUTE)
		
		set(cpp_file ${source_filename}.pb.cc)
		set(h_file ${source_filename}.pb.h)
		
		list(APPEND generated_header_files ${h_file})
		list(APPEND generated_source_files ${cpp_file})
		list(APPEND abs_path_generated_files ${CMAKE_CURRENT_SOURCE_DIR}/${cpp_file} ${CMAKE_CURRENT_SOURCE_DIR}/${h_file})
		list(APPEND abs_path_proto_files ${CMAKE_CURRENT_SOURCE_DIR}/${proto_file})
	endforeach(proto_file)
	add_custom_command(OUTPUT ${abs_path_generated_files} 
		COMMENT "Generating protobuf files..."
		COMMAND ${PROTOC} --proto_path=\"${CMAKE_CURRENT_SOURCE_DIR}\" --cpp_out=\"${CMAKE_CURRENT_SOURCE_DIR}\" ${abs_path_proto_files}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		DEPENDS ${abs_path_proto_files}
	)
	set(${var_generated_headers} ${generated_header_files} PARENT_SCOPE)
	set(${var_generated_source} ${generated_source_files} PARENT_SCOPE)
endfunction()

function(netcode_compile_shader var_binary_path source_path shader_type entry_point)
  get_filename_component(source_filename ${source_path} NAME_WE)
  get_filename_component(source_path "${source_path}" ABSOLUTE)
  set(binary_path "${CMAKE_BINARY_DIR}/Shaders/${source_filename}.cso")
  add_custom_command(OUTPUT ${binary_path}
    COMMENT "Generating ${binary_path}..."
    COMMAND ${FXC} /T \"${shader_type}\" /E\"${entry_point}\" /Od /Zi /Fo \"${binary_path}\" \"${source_path}\"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS "${source_path}"
  )
  set(${var_binary_path} "${binary_path}" PARENT_SCOPE)
endfunction()

function(netcode_add_cp_command source_path dest_file)
	get_filename_component(abs_path "${source_path}" ABSOLUTE)
	add_custom_command(OUTPUT ${dest_file}
		COMMENT "Copying ${source_path}..."
		COMMAND ${CMAKE_COMMAND} -E copy \"${abs_path}\" \"${dest_file}\"
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		DEPENDS "${abs_path}"
		USES_TERMINAL
	)
endfunction()

macro(netcode_increment_build_number dependent_target)
	IF(NOT DEFINED ENV{BUILD_NUMBER_INCREMENTED})
		set(ENV{BUILD_NUMBER_INCREMENTED} "TRUE")
		set(SRC_DIR "${PROJECT_SOURCE_DIR}/NetcodeFoundation")
		add_custom_command(TARGET ${dependent_target} PRE_BUILD COMMAND ${CMAKE_COMMAND} -DSRC_DIR="${SRC_DIR}" -DVERSIONED_BINARY="$<TARGET_FILE:${dependent_target}>" -P "${SRC_DIR}/increment_version.cmake")
	ENDIF()
endmacro()

set(NETCODE_SHADER_MODEL "5_1")

macro(NETCODE_COMPILE_VS outBinPath fileName)
	NETCODE_COMPILE_SHADER(${outBinPath} ${fileName} "vs_${NETCODE_SHADER_MODEL}" "main")
endmacro()

macro(NETCODE_COMPILE_PS outBinPath fileName)
	NETCODE_COMPILE_SHADER(${outBinPath} ${fileName} "ps_${NETCODE_SHADER_MODEL}" "main")
endmacro()

macro(NETCODE_COMPILE_GS outBinPath fileName)
	NETCODE_COMPILE_SHADER(${outBinPath} ${fileName} "gs_${NETCODE_SHADER_MODEL}" "main")
endmacro()

macro(NETCODE_COMPILE_HS outBinPath fileName)
	NETCODE_COMPILE_SHADER(${outBinPath} ${fileName} "hs_${NETCODE_SHADER_MODEL}" "main")
endmacro()

macro(NETCODE_COMPILE_DS outBinPath fileName)
	NETCODE_COMPILE_SHADER(${outBinPath} ${fileName} "ds_${NETCODE_SHADER_MODEL}" "main")
endmacro()

macro(NETCODE_COMPILE_CS outBinPath fileName)
	NETCODE_COMPILE_SHADER(${outBinPath} ${fileName} "cs_${NETCODE_SHADER_MODEL}" "main")
endmacro()

add_subdirectory("NetcodeFoundation")
add_subdirectory("NetcodeAssetLib")
add_subdirectory("NetcodeProtocol")
add_subdirectory("Netcode")
add_subdirectory("NetcodeClient")
add_subdirectory("NetcodeServer")

add_dependencies(Netcode NetcodeProtocol)
add_dependencies(NetcodeClient Netcode)
add_dependencies(NetcodeServer Netcode)
