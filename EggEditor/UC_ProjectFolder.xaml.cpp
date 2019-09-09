//
// UC_ProjectFolder.xaml.cpp
// Implementation of the UC_ProjectFolder class
//

#include "pch.h"
#include "UC_ProjectFolder.xaml.h"
#include "UC_Asset.xaml.h"
#include <collection.h>
#include <algorithm>
#include <string>

using namespace EggEditor;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The User Control item template is documented at https://go.microsoft.com/fwlink/?LinkId=234236

UC_ProjectFolder::UC_ProjectFolder(Platform::String^ folderName)
{
	InitializeComponent();
	ProjectFolderDataContext ^ pfdc = (ProjectFolderDataContext ^)DataContext;
	children = ref new Platform::Collections::Vector<Windows::UI::Xaml::UIElement ^>();
	pfdc->FolderName = folderName;
}

void UC_ProjectFolder::ProjectFolderDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args) {
	args->Data->SetText(L"Dummy");
	args->Data->RequestedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Move;
}

void EggEditor::UC_ProjectFolder::AssetRenamed(Windows::UI::Xaml::UIElement ^ asset) {
	unsigned int id;
	if(children->IndexOf(asset, &id)) {
		children->RemoveAt(id);
		InsertAsset(asset);
	}
	ContentChanged(this);
}

void EggEditor::UC_ProjectFolder::FolderRenamed(Windows::UI::Xaml::UIElement ^ projectFolder) {
	unsigned int id;
	if(children->IndexOf(projectFolder, &id)) {
		children->RemoveAt(id);
		InsertFolder(projectFolder);
	}
	ContentChanged(this);
}

void EggEditor::UC_ProjectFolder::InsertAsset(Windows::UI::Xaml::UIElement ^ asset) {
	UC_Asset ^ typedAsset = (UC_Asset ^)asset;

	for(unsigned int i = 0; i < children->Size; ++i) {
		Windows::UI::Xaml::UIElement ^ element = children->GetAt(i);

		if(element->GetType()->FullName == L"EggEditor.UC_Asset") {
			UC_Asset ^ assetToCompareTo = (UC_Asset ^)element;

			if(std::wcscmp(assetToCompareTo->TypedDataContext->AssetName->Data(), typedAsset->TypedDataContext->AssetName->Data()) > 0) {
				children->InsertAt(i, asset);
				return;
			}
		}
	}

	children->Append(asset);
}

void EggEditor::UC_ProjectFolder::InsertFolder(Windows::UI::Xaml::UIElement ^ folder) {
	UC_ProjectFolder ^ typedFolder = (UC_ProjectFolder ^)folder;

	for(unsigned int i = 0; i < children->Size; ++i) {
		Windows::UI::Xaml::UIElement ^ element = children->GetAt(i);

		// if we found a file, then insert it before the file
		if(element->GetType()->FullName != L"EggEditor.UC_ProjectFolder") {
			children->InsertAt(i, folder);
			return;
		}

		// if we found a folder, then compare the texts
		UC_ProjectFolder ^ folderToCompareTo = (UC_ProjectFolder ^)element;
		if(std::wcscmp(folderToCompareTo->TypedDataContext->FolderName->Data(), typedFolder->TypedDataContext->FolderName->Data()) > 0) {
			children->InsertAt(i, folder);
			return;
		}
	}

	// append if lookup failed
	children->Append(folder);
}


void EggEditor::UC_ProjectFolder::ProjectFolderDragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{
}

void EggEditor::UC_ProjectFolder::ProjectFolderDrop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{

}

void EggEditor::UC_ProjectFolder::TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	EggEditor::ProjectFolderDataContext ^ pfdc = (EggEditor::ProjectFolderDataContext ^)DataContext;
	if(e->Key == Windows::System::VirtualKey::Enter) {
		if(pfdc->FolderName == L"") {
			pfdc->FolderName = L"<Unnamed Folder>";
		}
		parentFolder->FolderRenamed(this);
		pfdc->EditState = 0;
	}
}

void EggEditor::UC_ProjectFolder::MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	EggEditor::ProjectFolderDataContext ^ pfdc = (EggEditor::ProjectFolderDataContext ^)DataContext;
	pfdc->EditState = 1;
}

void EggEditor::UC_ProjectFolder::TextBox_LostFocus(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	EggEditor::ProjectFolderDataContext ^ pfdc = (EggEditor::ProjectFolderDataContext ^)DataContext;
	pfdc->EditState = 0;
}

void EggEditor::UC_ProjectFolder::TextBox_FocusDisengaged(Windows::UI::Xaml::Controls::Control ^ sender, Windows::UI::Xaml::Controls::FocusDisengagedEventArgs ^ args)
{
	EggEditor::ProjectFolderDataContext ^ pfdc = (EggEditor::ProjectFolderDataContext ^)DataContext;
	pfdc->EditState = 0;
}
