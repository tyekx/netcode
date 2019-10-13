#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "UC_AssetEditor.g.h"

namespace winrt::EggEditor2::implementation
{
    struct UC_AssetEditor : UC_AssetEditorT<UC_AssetEditor>
    {
        UC_AssetEditor();

        int32_t MyProperty();
        void MyProperty(int32_t value);

        void ClickHandler(Windows::Foundation::IInspectable const& sender, Windows::UI::Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::EggEditor2::factory_implementation
{
    struct UC_AssetEditor : UC_AssetEditorT<UC_AssetEditor, implementation::UC_AssetEditor>
    {
    };
}
