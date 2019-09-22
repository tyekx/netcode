//
// UC_Asset.xaml.cpp
// Implementation of the UC_Asset class
//

#include "pch.h"
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

UC_Asset ^ UC_Asset::currentlyDragging{ nullptr };


UC_Asset::UC_Asset()
{
	InitializeComponent();
}

UC_Asset::UC_Asset(Platform::String ^ text, UINT atype)
{
	InitializeComponent();
	TypedDataContext->Name = text;
	TypedDataContext->AssetType = (int)atype;
}


void EggEditor::UC_Asset::AssetDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args)
{
	/*
	Platform::String ^ prefix;

	switch(assetType) {
	case ASSET_TYPE_SHADED_MESH:
		prefix = L"/SM/";
		break;
	case ASSET_TYPE_MATERIAL:
		prefix = L"/MAT/";
		break;
	case ASSET_TYPE_TEXTURE2D:
		prefix = L"/TEX2D/";
		break;
	case ASSET_TYPE_MESH:
		prefix = L"/MESH/";
		break;
	}*/



	currentlyDragging = this;
	args->Data->RequestedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link |
									Windows::ApplicationModel::DataTransfer::DataPackageOperation::Move;
}

void EggEditor::UC_Asset::MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	TypedDataContext->UI->EditState = 1;
}


void EggEditor::UC_Asset::TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	if(e->Key == Windows::System::VirtualKey::Enter) {
		TypedDataContext->UI->EditState = 0;
	}
}


void EggEditor::UC_Asset::CtxOpenInEditorBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	OpenAsset(this);
}
