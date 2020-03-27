#pragma once
#include "UC_Lod.g.h"
#include "DC_Lod.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct UC_Lod : UC_LodT<UC_Lod>
    {
        UC_Lod();

        UC_Lod(NetcodeAssetEditor::DC_Lod const& myContent);
        NetcodeAssetEditor::DC_Lod MyContent();
        void MyContent(NetcodeAssetEditor::DC_Lod const& value);
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct UC_Lod : UC_LodT<UC_Lod, implementation::UC_Lod>
    {
    };
}
