//
// UC_MeshComponent.xaml.cpp
// Implementation of the UC_MeshComponent class
//

#include "pch.h"
#include "UC_MeshComponent.xaml.h"
#include "Auxiliary.h"
#include "UC_Asset.xaml.h"

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

UC_MeshComponent::UC_MeshComponent()
{
	InitializeComponent();
}


void EggEditor::UC_MeshComponent::ScriptRefTextBox_DragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{
	e->AcceptedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link;
}


void EggEditor::UC_MeshComponent::ScriptRefTextBox_Drop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{
	if(UC_Asset::CurrentlyDragging) {
		DataContext = UC_Asset::CurrentlyDragging->TypedDataContext;
	}
}
