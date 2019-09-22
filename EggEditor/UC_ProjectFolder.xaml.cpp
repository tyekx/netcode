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

UC_ProjectFolder::UC_ProjectFolder()
{
	InitializeComponent();
}

void UC_ProjectFolder::ProjectFolderDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args) {
	args->Data->SetText(L"Dummy");
	args->Data->RequestedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Move;
}


void EggEditor::UC_ProjectFolder::ProjectFolderDragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{
}

void EggEditor::UC_ProjectFolder::ProjectFolderDrop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{

}

void EggEditor::UC_ProjectFolder::TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	ProjectFolderDataContext ^ pfdc = (ProjectFolderDataContext ^)DataContext;

	if(e->Key == Windows::System::VirtualKey::Enter) {
		if(TypedDataContext->Name == L"") {
			TypedDataContext->Name = L"<Unnamed Folder>";
		}
		TypedDataContext->UI->EditState = 0;
	}
}

void EggEditor::UC_ProjectFolder::MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	TypedDataContext->UI->EditState = 1;
}

void EggEditor::UC_ProjectFolder::TextBox_LostFocus(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	TypedDataContext->UI->EditState = 0;
}

void EggEditor::UC_ProjectFolder::TextBox_FocusDisengaged(Windows::UI::Xaml::Controls::Control ^ sender, Windows::UI::Xaml::Controls::FocusDisengagedEventArgs ^ args)
{
	TypedDataContext->UI->EditState = 0;
}
