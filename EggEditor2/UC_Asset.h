#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "UC_Asset.g.h"
#include "AssetDataContext.h"
#include "AssetUIDataContext.h"
#include "UC_EditableTextBlock.h"

namespace winrt::EggEditor2::implementation
{
    struct UC_Asset : UC_AssetT<UC_Asset>
    {
        UC_Asset();

    };
}

namespace winrt::EggEditor2::factory_implementation
{
    struct UC_Asset : UC_AssetT<UC_Asset, implementation::UC_Asset>
    {
    };
}
