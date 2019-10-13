#include "pch.h"
#include "UC_AssetBrowser.h"
#if __has_include("UC_AssetBrowser.g.cpp")
#include "UC_AssetBrowser.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggEditor2::implementation
{
    UC_AssetBrowser::UC_AssetBrowser()
    {
        InitializeComponent();
    }

	void UC_AssetBrowser::BtnTest(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e) {

	}
}
