//
// UC_ProjectFolder.xaml.h
// Declaration of the UC_ProjectFolder class
//

#pragma once

#include "Auxiliary.h"
#include "UC_ProjectFolder.g.h"

namespace EggEditor
{


	public delegate void ContentChangedCallback(Platform::Object ^ sender);

	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class UC_ProjectFolder sealed 
	{
		UC_ProjectFolder ^ parentFolder;
		Platform::Collections::Vector<Windows::UI::Xaml::UIElement ^> ^ children;


	public:
		event EggEditor::ContentChangedCallback ^ ContentChanged;

		UC_ProjectFolder(Platform::String^ folderName);
		void ProjectFolderDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args);

		property Windows::Foundation::Collections::IObservableVector<Windows::UI::Xaml::UIElement ^> ^ Children {
			Windows::Foundation::Collections::IObservableVector<Windows::UI::Xaml::UIElement ^> ^ get() {
				return children;
			}
		}

		void AssetRenamed(Windows::UI::Xaml::UIElement ^ asset);
		void FolderRenamed(Windows::UI::Xaml::UIElement ^ folder);

		void InsertAsset(Windows::UI::Xaml::UIElement ^ asset);
		void InsertFolder(Windows::UI::Xaml::UIElement ^ folder);

		property UC_ProjectFolder ^ ParentFolder {
			UC_ProjectFolder ^ get() {
				return parentFolder;
			}
			void set(UC_ProjectFolder ^ pf) {
				if(parentFolder) {
					unsigned int id;
					if(parentFolder->Children->IndexOf(this, &id)) {
						parentFolder->Children->RemoveAt(id);
					}
				}
				parentFolder = pf;
				parentFolder->InsertFolder(this);
			}
		}

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
