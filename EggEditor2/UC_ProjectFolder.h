#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "UC_ProjectFolder.g.h"

namespace winrt::EggEditor2::implementation
{
    struct UC_ProjectFolder : UC_ProjectFolderT<UC_ProjectFolder>
    {
        UC_ProjectFolder();
    };
}

namespace winrt::EggEditor2::factory_implementation
{
    struct UC_ProjectFolder : UC_ProjectFolderT<UC_ProjectFolder, implementation::UC_ProjectFolder>
    {
    };
}
