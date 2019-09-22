//
// UC_ProjectFolder.xaml.h
// Declaration of the UC_ProjectFolder class
//

#pragma once

#include "Auxiliary.h"
#include "UC_ProjectFolder.g.h"
#include "ProjectFolderDataContext.h"

namespace EggEditor
{


	public delegate void ContentChangedCallback(Platform::Object ^ sender);

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_ProjectFolder sealed 
	{
	public:
		event EggEditor::ContentChangedCallback ^ ContentChanged;

		UC_ProjectFolder();

		void ProjectFolderDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args);

		property ProjectFolderDataContext ^ TypedDataContext {
			ProjectFolderDataContext ^ get() {
				return (ProjectFolderDataContext ^)DataContext;
			}
		}

	private:
		void ProjectFolderDragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
		void ProjectFolderDrop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e);
		void TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e);
		void MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void TextBox_LostFocus(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e);
		void TextBox_FocusDisengaged(Windows::UI::Xaml::Controls::Control ^ sender, Windows::UI::Xaml::Controls::FocusDisengagedEventArgs ^ args);
	};
}
