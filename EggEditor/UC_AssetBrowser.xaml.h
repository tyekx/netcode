//
// UC_AssetBrowser.xaml.h
// Declaration of the UC_AssetBrowser class
//

#pragma once

#include "UC_AssetBrowser.g.h"
#include "UC_ProjectFolder.xaml.h"

namespace EggEditor
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_AssetBrowser sealed
	{
		UC_ProjectFolder ^ rootFolder;
		UC_ProjectFolder ^ currentFolder;

	public:
		UC_AssetBrowser();


	private:
		void CtxBtnNewProjectFolder_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void CtxBtnNewMaterial_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void CtxBtnNewShadedMesh_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);


		void BreadCrumb_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::Input::TappedRoutedEventArgs ^ e);
		void ProjectFolder_OnContentChanged(Platform::Object ^ sender);

		void ProjectFolder_DoubleTap(Platform::Object ^ sender, Windows::UI::Xaml::Input::DoubleTappedRoutedEventArgs ^ e);
		void ChangeProjectFolder(UC_ProjectFolder ^ folder);
		/*
		this function is a helper function for ChangeProjectFolder, do not call it
		*/
		void ImplDetail_RecursiveAddBreadCrumb(UC_ProjectFolder ^ folder);
	};
}
