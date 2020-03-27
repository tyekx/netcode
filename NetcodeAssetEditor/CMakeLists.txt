cmake_minimum_required(VERSION 3.8)

add_compile_options(-Wno-delete-non-virtual-dtor -mcx16 -fno-delayed-template-parsing -Xclang -fcoroutines-ts -std:c++latest)

set(SRC_FILES
    AnimationsPage.cpp
    App.cpp
    BonesPage.cpp
    CollidersPage.cpp
    ColliderTypeConverter.cpp
    ColliderTypeVisibilityConverter.cpp
    CreateColliderDialog.cpp
    DC_Animation.cpp
    DC_Bone.cpp
    DC_Collider.cpp
    DC_Lod.cpp
    DC_MainPage.cpp
    DC_Material.cpp
    DC_Mesh.cpp
    DC_Vector2Control.cpp
    DC_Vector3Control.cpp
    DC_Vector4Control.cpp
    DepthToMarginConverter.cpp
    DX12GraphicsModuleForXaml.cpp
    EditorModuleFactory.cpp
    EggAssetExporter.cpp
    FBXImporter.cpp
    FloatBox.cpp
    GeometryPage.cpp
    MainPage.cpp
    MaterialsPage.cpp
    pch.cpp
    SelectionToVisibilityConverter.cpp
    UC_Lod.cpp
    UC_Mesh.cpp
    Vector2Control.cpp
    Vector3Control.cpp
    Vector4Control.cpp
    VectorConverter.cpp
    XamlGlobal.cpp
    XamlHelpers.cpp
)

set(HEADER_FILES
    AnimationsPage.h
    App.h
    BonesPage.h
    BoundingBoxHelpers.h
    CollidersPage.h
    ColliderTypeConverter.h
    ColliderTypeVisibilityConverter.h
    ConstantBufferTypes.h
    CreateColliderDialog.h
    DC_Animation.h
    DC_Bone.h
    DC_Collider.h
    DC_Lod.h
    DC_MainPage.h
    DC_Material.h
    DC_Mesh.h
    DC_Vector2Control.h
    DC_Vector3Control.h
    DC_Vector4Control.h
    DepthToMarginConverter.h
    DX12GraphicsModuleForXaml.h
    EditorApp.h
    EditorFrameGraph.h
    EditorModuleFactory.h
    EggAssetExporter.h
    FBXImporter.h
    FloatBox.h
    GBuffer.h
    GeometryPage.h
    MainPage.h
    MaterialsPage.h
    Model.h
    pch.h
    SelectionToVisibilityConverter.h
    UC_Lod.h
    UC_Mesh.h
    Vector2Control.h
    Vector3Control.h
    Vector4Control.h
    VectorConverter.h
    XamlGlobal.h
    XamlHelpers.h
)

set(XAML_FILES
    AnimationsPage.xaml
    App.xaml
    BonesPage.xaml
    CollidersPage.xaml
    CreateColliderDialog.xaml
    GeometryPage.xaml
    MainPage.xaml
    MaterialsPage.xaml
    UC_Lod.xaml
    UC_Mesh.xaml
    Vector2Control.xaml
    Vector3Control.xaml
    Vector4Control.xaml
)

set(IDL_FILES
    App.idl
    BonesPage.idl
    GeometryPage.idl
    MainPage.idl
    Vector2Control.idl
    Vector3Control.idl
    Vector4Control.idl
)

set(ASSET_FILES
    Assets/LockScreenLogo.scale-200.png
    Assets/SplashScreen.scale-200.png
    Assets/Square44x44Logo.scale-200.png
    Assets/Square44x44Logo.targetsize-24_altform-unplated.png
    Assets/Square150x150Logo.scale-200.png
    Assets/StoreLogo.png
    Assets/Wide310x150Logo.scale-200.png
    Assets/loading_icon.png
)

set(CONTENT_FILES
    Package.appxmanifest
)

set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")

set(RESOURCE_FILES
  ${CONTENT_FILES} ${ASSET_FILES}
  "${CMAKE_BINARY_DIR}/packages.config"
  EggAssetEditor_TemporaryKey.pfx)
  

set_property(SOURCE "App.xaml" PROPERTY VS_XAML_TYPE "ApplicationDefinition")

add_executable(NetcodeAssetEditor ${SRC_FILES} ${HEADER_FILES} ${RESOURCE_FILES} ${XAML_FILES} ${IDL_FILES})

set_target_properties(NetcodeAssetEditor PROPERTIES VS_GLOBAL_ROOTNAMESPACE Egg)
set_target_properties(NetcodeAssetEditor PROPERTIES VS_CPPWINRT_EXTENSIONS true)


target_link_libraries(NetcodeAssetEditor ole32 windowsapp)

function(ADD_OBJECT_DEPENDS file dependency)
    get_source_file_property(cur_obj_depends ${file} OBJECT_DEPENDS)
    if (${cur_obj_depends} STREQUAL "NOTFOUND")
        set(new_obj_depends "${pch_output_file_name}")
    else()
        set(new_obj_depends "${cur_obj_depends};${dependency}")
    endif()
    set_source_files_properties(${file}  PROPERTIES OBJECT_DEPENDS ${new_obj_depends})
endfunction(ADD_OBJECT_DEPENDS)

function(TARGET_CONFIG_MSVC_PCH target pch_cpp pch_header)
    get_target_property(target_name ${target} NAME)
    set(pch_output_file_name "${target_name}.pch")
    set(pch_output_file "\"${CMAKE_CURRENT_BINARY_DIR}/${pch_output_file_name}\"")

    get_target_property(sources ${target} SOURCES)
    foreach(file ${sources})
        if (${file} STREQUAL ${pch_cpp})
            set_source_files_properties(${file}
                PROPERTIES
                COMPILE_FLAGS " /Yc${pch_header} /Fp${pch_output_file}"
                OBJECT_OUTPUTS ${pch_output_file_name})
        else()
            set_source_files_properties(${file} PROPERTIES COMPILE_FLAGS " /Yu${pch_header} /Fp${pch_output_file}")
            ADD_OBJECT_DEPENDS(${file} ${pch_output_file_name})
        endif()
    endforeach()

    set_target_properties(${target} PROPERTIES PCH_OUTPUT_FILE ${pch_output_file})
    set_property(GLOBAL APPEND PROPERTY PCH_OUTPUT_FILES ${pch_output_file})
endfunction(TARGET_CONFIG_MSVC_PCH)

target_config_msvc_pch(NetcodeAssetEditor "pch.cpp" "pch.h")
