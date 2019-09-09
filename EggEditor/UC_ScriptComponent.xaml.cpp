//
// UC_ScriptComponent.xaml.cpp
// Implementation of the UC_ScriptComponent class
//

#include "pch.h"
#include "UC_ScriptComponent.xaml.h"

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

UC_ScriptComponent::UC_ScriptComponent()
{
	InitializeComponent();
}


void EggEditor::UC_ScriptComponent::TextBox_DragOver(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{
	e->AcceptedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link;
}


void EggEditor::UC_ScriptComponent::TextBox_Drop(Platform::Object ^ sender, Windows::UI::Xaml::DragEventArgs ^ e)
{

	auto asyncData = e->DataView->GetTextAsync();

	auto itemsTask = concurrency::create_task(asyncData);

	itemsTask.then([this](Platform::String ^ r) {
		scriptRefTextBox->Text = r;
    });
}
