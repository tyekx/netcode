//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "ggl005App.h"
#include "MainPage.g.h"
#include "Auxiliary.h"
#include <Egg/Stopwatch.h>
#include "UC_ProjectFolder.xaml.h"

namespace EggEditor
{
	public ref class MainPage sealed 
	{
		std::unique_ptr<ggl005App> gameApp;
		Egg::Stopwatch stopwatch;
		float totalTime;
		
		Windows::UI::Xaml::DispatcherTimer^ timer;
		float swpWidth;
		float swpHeight;
	public:
		MainPage();

	private:
		void Page_Loaded(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void DispatcherTimer_Tick(Platform::Object ^ sender, Platform::Object ^ e);
		void SwapChainPanel_SizeChanged(Platform::Object ^ sender, Windows::UI::Xaml::SizeChangedEventArgs ^ e);
		void SwapChainPanel_PointerPressed(Platform::Object ^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs ^ e);
		void BtnOutput_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void BtnAssets_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void BtnPreview_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void BtnAssetEditor_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void BtnStart_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void BtnPause_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void BtnReset_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);


		void FileMenuBtnNew_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void EditMenuBtnImport_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void OnOpenAsset(Platform::Object ^ asset);


	};
}
