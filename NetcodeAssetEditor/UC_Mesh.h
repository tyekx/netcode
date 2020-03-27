#pragma once

#include "DC_Mesh.h"
#include "UC_Lod.h"
#include "DC_Lod.h"
#include "UC_Mesh.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct UC_Mesh : UC_MeshT<UC_Mesh>
    {
        UC_Mesh();

    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct UC_Mesh : UC_MeshT<UC_Mesh, implementation::UC_Mesh>
    {
    };
}
