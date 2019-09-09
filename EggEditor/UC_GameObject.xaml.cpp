//
// UC_GameObject.xaml.cpp
// Implementation of the UC_GameObject class
//

#include "pch.h"
#include "UC_GameObject.xaml.h"
#include <Egg/Utility.h>

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

UC_GameObject::UC_GameObject(UINT_PTR gameObjectPtr)
{
	InitializeComponent();
	TypedDataContext = (GameObjectDataContext ^)DataContext;
	goRef = reinterpret_cast<Egg::GameObject *>(gameObjectPtr);
	TypedDataContext->SetGameObject(reinterpret_cast<UINT_PTR>(goRef));
}

void EggEditor::UC_GameObject::Grid_DragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{
	e->AcceptedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link;
}


void EggEditor::UC_GameObject::Grid_Drop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{
}


void EggEditor::UC_GameObject::Grid_DragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args)
{
	args->Data->RequestedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link;
	args->Data->SetText(L"Test Value");
}

void EggEditor::UC_GameObject::CtxRenameBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	TypedDataContext->EditState = 1;
}


void EggEditor::UC_GameObject::TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	if(e->Key == Windows::System::VirtualKey::Enter) {
		TypedDataContext->EditState = 0;
	}
}
