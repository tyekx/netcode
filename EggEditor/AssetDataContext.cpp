#include "pch.h"
#include "AssetDataContext.h"

void EggEditor::AssetDataContext::SetAssetType(int value) {
	switch(assetType) {
	case ASSET_TYPE_SHADED_MESH:
		uiContext->OverlayText = L"SM";
		uiContext->GradientStart = Windows::UI::Color{ 255, 238, 191, 227 };
		uiContext->GradientStop = Windows::UI::Color{ 255, 112, 61, 105 };
		uiContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Page;
		break;
	case ASSET_TYPE_MATERIAL:
		uiContext->OverlayText = L"MAT";
		uiContext->GradientStart = Windows::UI::Color{ 255, 110, 220, 170 };
		uiContext->GradientStop = Windows::UI::Color{ 255, 70, 200, 190 };
		uiContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Page;
		break;
	case ASSET_TYPE_TEXTURE2D:
		uiContext->OverlayText = L"TEX2D";
		uiContext->GradientStart = Windows::UI::Color{ 255, 255, 160, 139 };
		uiContext->GradientStop = Windows::UI::Color{ 255, 232, 120, 116 };
		uiContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Pictures;
		break;
	case ASSET_TYPE_MESH:
		uiContext->OverlayText = L"MESH";
		uiContext->GradientStart = Windows::UI::Color{ 255, 255, 160, 139 };
		uiContext->GradientStop = Windows::UI::Color{ 255, 232, 120, 116 };
		uiContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::ViewAll;
		break;
	case ASSET_TYPE_SHADER:
		uiContext->OverlayText = L"SHADER";
		uiContext->GradientStart = Windows::UI::Color{ 255, 255, 160, 139 };
		uiContext->GradientStop = Windows::UI::Color{ 255, 232, 120, 116 };
		uiContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Edit;
		break;
	default:
		uiContext->AssetIcon = Windows::UI::Xaml::Controls::Symbol::Help;
		uiContext->OverlayText = L"";
		uiContext->GradientStart = Windows::UI::Color{ 255, 207, 207, 207 };
		uiContext->GradientStop = Windows::UI::Color{ 255, 160, 160, 160 };
		break;
	}
}
