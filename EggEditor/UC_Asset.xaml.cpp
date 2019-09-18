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

UC_Asset::UC_Asset(Platform::String ^ text, UINT atype)
{
	InitializeComponent();
	assetType = atype;
	TypedDataContext->AssetName = text;
	switch(assetType) {
	case ASSET_TYPE_SHADED_MESH: 
		TypedDataContext->OverlayText = L"SM";
		TypedDataContext->GradientStart = Windows::UI::Color{ 255, 238, 191, 227 };
		TypedDataContext->GradientStop = Windows::UI::Color{ 255, 112, 61, 105 };
		TypedDataContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Page;
		break;
	case ASSET_TYPE_MATERIAL:
		TypedDataContext->OverlayText = L"MAT";
		TypedDataContext->GradientStart = Windows::UI::Color{ 255, 110, 220, 170 };
		TypedDataContext->GradientStop = Windows::UI::Color{ 255, 70, 200, 190};
		TypedDataContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Page;
		break;
	case ASSET_TYPE_TEXTURE2D:
		TypedDataContext->OverlayText = L"TEX2D";
		TypedDataContext->GradientStart = Windows::UI::Color{ 255, 255, 160, 139 };
		TypedDataContext->GradientStop = Windows::UI::Color{ 255, 232, 120, 116 };
		TypedDataContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Pictures;
		break;
	case ASSET_TYPE_MESH:
		TypedDataContext->OverlayText = L"MESH";
		TypedDataContext->GradientStart = Windows::UI::Color{ 255, 255, 160, 139 };
		TypedDataContext->GradientStop = Windows::UI::Color{ 255, 232, 120, 116 };
		TypedDataContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::ViewAll;
		break;
	case ASSET_TYPE_SHADER:
		TypedDataContext->OverlayText = L"SHADER";
		TypedDataContext->GradientStart = Windows::UI::Color{ 255, 255, 160, 139 };
		TypedDataContext->GradientStop = Windows::UI::Color{ 255, 232, 120, 116 };
		TypedDataContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Edit;
		break;
	default:
		TypedDataContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Help;
		TypedDataContext->OverlayText = L"";
		TypedDataContext->GradientStart = Windows::UI::Color{ 255, 207, 207, 207 };
		TypedDataContext->GradientStop = Windows::UI::Color{ 255, 160, 160, 160};
		break;
	}


}


void EggEditor::UC_Asset::AssetDragStarting(Windows::UI::Xaml::UIElement ^ sender, Windows::UI::Xaml::DragStartingEventArgs ^ args)
{
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
	}



	currentlyDragging = this;
	args->Data->RequestedOperation = Windows::ApplicationModel::DataTransfer::DataPackageOperation::Link |
									Windows::ApplicationModel::DataTransfer::DataPackageOperation::Move;
}




void EggEditor::UC_Asset::ImplDetail_InsertAssetIntoParent() {
	parentFolder->InsertAsset(this);
}


void EggEditor::UC_Asset::MenuFlyoutItem_Click(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	TypedDataContext->EditState = 1;
}


void EggEditor::UC_Asset::TextBox_KeyDown(Platform::Object ^ sender, Windows::UI::Xaml::Input::KeyRoutedEventArgs ^ e)
{
	if(e->Key == Windows::System::VirtualKey::Enter) {
		TypedDataContext->EditState = 0;
	}
}


void EggEditor::UC_Asset::CtxOpenInEditorBtn_OnClick(Platform::Object ^ sender, Windows::UI::Xaml::RoutedEventArgs ^ e)
{
	OpenAsset(this);
}
