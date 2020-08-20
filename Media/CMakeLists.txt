cmake_minimum_required(VERSION 3.8)

find_program(TEXCONV texconv PATHS "${PROJECT_SOURCE_DIR}/Tools/dxtex")

if(NOT TEXCONV)
	message(FATAL_ERROR "TexConv not found")
endif()

find_program(SPRITE_FONT MakeSpriteFont PATHS "${PROJECT_SOURCE_DIR}/Tools/dxtex")

if(NOT SPRITE_FONT)
    message(FATAL_ERROR "MakeSpriteFont was not found")
endif()

function(netcode_compile_texture_func var_output_path source_path mip_levels texture_format)
    get_filename_component(source_filename ${source_path} NAME_WE)
    get_filename_component(source_abs_path ${source_path} ABSOLUTE)
    get_filename_component(source_reldir ${source_abs_path} DIRECTORY)
    file(RELATIVE_PATH source_reldir ${CMAKE_CURRENT_LIST_DIR} ${source_reldir})
    set(output_dir "${CMAKE_CURRENT_LIST_DIR}/compiled/${source_reldir}")
    set(output_path "${output_dir}/${source_filename}.dds")
    add_custom_command(OUTPUT ${output_path}
        COMMENT "Compiling texture: ${output_path}..."
        COMMAND ${TEXCONV} -f "${texture_format}" -nologo -ft dds -m "${mip_levels}" -y -o "compiled" "${source_path}"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        DEPENDS ${source_path}
    )
    list(APPEND $)
    set(${var_output_path} "${output_path}" PARENT_SCOPE)
endfunction()

set(NETCODE_ASSETS "")

macro(netcode_compile_texture)
    netcode_compile_texture_func(TOUT ${ARGV})
    list(APPEND NETCODE_ASSETS ${TOUT})
endmacro()

macro(netcode_copy_texture src_path)
    get_filename_component(source_filename ${src_path} NAME)
    get_filename_component(source_abs_path ${src_path} ABSOLUTE)
    get_filename_component(source_reldir ${source_abs_path} DIRECTORY)
    file(RELATIVE_PATH source_reldir ${CMAKE_CURRENT_LIST_DIR} ${source_reldir})
    set(output_path "${CMAKE_CURRENT_LIST_DIR}/compiled/${source_reldir}/${source_filename}")
    netcode_add_cp_command(${src_path} ${output_path})
    list(APPEND NETCODE_ASSETS ${output_path})
endmacro()

netcode_compile_texture("textures/scifi_panel/Scifi_Panel_1K_albedo.tif" 11 BC7_UNORM)
netcode_compile_texture("textures/scifi_panel/Scifi_Panel_1K_normal.tif" 11 BC6H_UF16)
netcode_compile_texture("textures/scifi_panel/Scifi_Panel_1K_ao.tif" 11 BC4_UNORM)
netcode_compile_texture("textures/scifi_panel/Scifi_Panel_1K_height.tif" 11 BC4_UNORM)
netcode_compile_texture("textures/scifi_panel/Scifi_Panel_1K_metallic.tif" 11 BC4_UNORM)
netcode_compile_texture("textures/scifi_panel/Scifi_Panel_1K_roughness.tif" 11 BC4_UNORM)

netcode_compile_texture("textures/brick_1/Wall_Stone3_3x3_1K_albedo.tif" 11 BC7_UNORM)
netcode_compile_texture("textures/brick_1/Wall_Stone3_3x3_1K_normal.tif" 11 BC6H_UF16)
netcode_compile_texture("textures/brick_1/Wall_Stone3_3x3_1K_height.tif" 11 BC4_UNORM)
netcode_compile_texture("textures/brick_1/Wall_Stone3_3x3_1K_ao.tif" 11 BC4_UNORM)
netcode_compile_texture("textures/brick_1/Wall_Stone3_3x3_1K_roughness.tif" 11 BC4_UNORM)

netcode_compile_texture("textures/chest_01/chest_01_diff_2k.png" 11 BC7_UNORM)
netcode_compile_texture("textures/chest_01/chest_01_nrm_2k.png" 11 BC6H_UF16)
netcode_compile_texture("textures/chest_01/chest_01_metal_2k.png" 11 BC4_UNORM)
netcode_compile_texture("textures/chest_01/chest_01_ao_2k.png" 11 BC4_UNORM)
netcode_compile_texture("textures/chest_01/chest_01_rough_2k.png" 11 BC4_UNORM)

netcode_compile_texture("textures/envmaps/cloudynoon.dds" 10 BC7_UNORM)

netcode_copy_texture("textures/ui/aenami_dreamer.jpg")
netcode_copy_texture("textures/ui/loading_icon.png")

add_custom_target(NetcodeAssets DEPENDS ${NETCODE_ASSETS})

