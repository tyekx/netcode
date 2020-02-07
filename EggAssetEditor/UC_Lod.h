#pragma once
#include "UC_Lod.g.h"
#include "DC_Lod.h"

namespace winrt::EggAssetEditor::implementation
{
    struct UC_Lod : UC_LodT<UC_Lod>
    {
        UC_Lod();

        UC_Lod(EggAssetEditor::DC_Lod const& myContent);
        EggAssetEditor::DC_Lod MyContent();
        void MyContent(EggAssetEditor::DC_Lod const& value);
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct UC_Lod : UC_LodT<UC_Lod, implementation::UC_Lod>
    {
    };
}
