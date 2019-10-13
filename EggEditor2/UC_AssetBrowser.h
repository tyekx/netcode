#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "UC_AssetBrowser.g.h"
#include "AssetBrowserDataContext.h"

namespace winrt::EggEditor2::implementation
{
    struct UC_AssetBrowser : UC_AssetBrowserT<UC_AssetBrowser>
    {
        UC_AssetBrowser();
		void BtnTest(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
    };
}

namespace winrt::EggEditor2::factory_implementation
{
    struct UC_AssetBrowser : UC_AssetBrowserT<UC_AssetBrowser, implementation::UC_AssetBrowser>
    {
    };
}
