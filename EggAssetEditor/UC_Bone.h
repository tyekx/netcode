#pragma once

#include "DC_Bone.h"
#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "UC_Bone.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct UC_Bone : UC_BoneT<UC_Bone>
    {
        UC_Bone();
    };
}

namespace winrt::EggAssetEditor::factory_implementation
{
    struct UC_Bone : UC_BoneT<UC_Bone, implementation::UC_Bone>
    {
    };
}
