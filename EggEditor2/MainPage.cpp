#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggEditor2::implementation
{
    MainPage::MainPage()
    {
        InitializeComponent();
    }
	
	void MainPage::BtnAssetEditor_OnClick(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::UI::Xaml::RoutedEventArgs const &)
	{
		TypedDataContext().MainWindowState(1);
	}

	void MainPage::BtnWorld_OnClick(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::UI::Xaml::RoutedEventArgs const &) {
		TypedDataContext().MainWindowState(0);
	}

	void MainPage::BtnConsole_OnClick(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::UI::Xaml::RoutedEventArgs const &) {
		TypedDataContext().SubWindowState(0);
	}

	void MainPage::BtnAssetBrowser_OnClick(winrt::Windows::Foundation::IInspectable const &, winrt::Windows::UI::Xaml::RoutedEventArgs const &) {
		TypedDataContext().SubWindowState(1);
	}
}



