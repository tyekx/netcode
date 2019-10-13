#pragma once

#include "MainPage.g.h"
#include "MainPageDataContext.h"
#include "ProjectDataContext.h"
#include "ProjectFolderDataContext.h"
#include "ProjectFolderUIDataContext.h"
#include "UC_Asset.h"

namespace winrt::EggEditor2::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
        MainPage();
		void BtnAssetEditor_OnClick(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
		void BtnWorld_OnClick(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
		void BtnConsole_OnClick(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
		void BtnAssetBrowser_OnClick(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
	};
}

namespace winrt::EggEditor2::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
