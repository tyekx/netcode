#include "pch.h"
#include "AssetDataContext.h"
#include "AssetDataContext.g.cpp"

namespace winrt::EggEditor2::implementation
{

	void AssetDataContext::SetAssetType(int assetType) {
		switch(assetType) {
		case ASSET_TYPE_SHADED_MESH:
			uiContext.OverlayText(L"SM");
			uiContext.GradientStart(Windows::UI::Color{ 255, 238, 191, 227 });
			uiContext.GradientStop(Windows::UI::Color{ 255, 112, 61, 105 });
			uiContext.AssetIcon(Windows::UI::Xaml::Controls::Symbol::Page);
			break;
		case ASSET_TYPE_MATERIAL:
			uiContext.OverlayText(L"MAT");
			uiContext.GradientStart(Windows::UI::Color{ 255, 110, 220, 170 });
			uiContext.GradientStop(Windows::UI::Color{ 255, 70, 200, 190 });
			uiContext.AssetIcon(Windows::UI::Xaml::Controls::Symbol::Page);
			break;
		case ASSET_TYPE_TEXTURE2D:
			uiContext.OverlayText(L"TEX2D");
			uiContext.GradientStart(Windows::UI::Color{ 255, 255, 160, 139 });
			uiContext.GradientStop(Windows::UI::Color{ 255, 232, 120, 116 });
			uiContext.AssetIcon(Windows::UI::Xaml::Controls::Symbol::Pictures);
			break;
		case ASSET_TYPE_MESH:
			uiContext.OverlayText(L"MESH");
			uiContext.GradientStart(Windows::UI::Color{ 255, 255, 160, 139 });
			uiContext.GradientStop(Windows::UI::Color{ 255, 232, 120, 116 });
			uiContext.AssetIcon(Windows::UI::Xaml::Controls::Symbol::ViewAll);
			break;
		case ASSET_TYPE_SHADER:
			uiContext.OverlayText(L"SHADER");
			uiContext.GradientStart(Windows::UI::Color{ 255, 255, 160, 139 });
			uiContext.GradientStop(Windows::UI::Color{ 255, 232, 120, 116 });
			uiContext.AssetIcon(Windows::UI::Xaml::Controls::Symbol::Edit);
			break;
		default:
			uiContext.AssetIcon(Windows::UI::Xaml::Controls::Symbol::Help);
			uiContext.OverlayText(L"");
			uiContext.GradientStart(Windows::UI::Color{ 255, 207, 207, 207 });
			uiContext.GradientStop(Windows::UI::Color{ 255, 160, 160, 160 });
			break;
		}
	}

	AssetDataContext::AssetDataContext(int32_t type) : AssetDataContext() {
		SetAssetType(type);
	}

    int32_t AssetDataContext::Type()
    {
		return type;
    }

    void AssetDataContext::Type(int32_t value)
    {
		if(type != value) {
			type = value;
			SetAssetType(type);
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"Type" });
		}
    }

    EggEditor2::AssetUIDataContext AssetDataContext::UI()
    {
		return uiContext;
    }

    winrt::event_token AssetDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
		return propertyChanged.add(handler);
    }

    void AssetDataContext::PropertyChanged(winrt::event_token const& token) noexcept
    {
		propertyChanged.remove(token);
    }

    hstring AssetDataContext::Name()
    {
		return name;
    }

    void AssetDataContext::Name(hstring const& value)
    {
		if(name != value) {
			name = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"Name" });
		}
    }

    EggEditor2::IAsset AssetDataContext::ParentFolder()
    {
		return parentFolder;
    }

    void AssetDataContext::ParentFolder(EggEditor2::IAsset const& value)
    {
		if(parentFolder != value) {
			parentFolder = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"ParentFolder" });
		}
    }

    bool AssetDataContext::IsDirty()
    {
		return isDirty;
    }

    void AssetDataContext::IsDirty(bool value)
    {
		if(isDirty != value) {
			isDirty = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"IsDirty" });
		}
    }

    bool AssetDataContext::IsAsset()
    {
		return true;
    }

    bool AssetDataContext::IsContentDirty()
    {
		return IsDirty();
    }

    Windows::Foundation::Collections::IObservableVector<EggEditor2::IAsset> AssetDataContext::Children()
    {
		throw hresult_not_implemented();
    }

    void AssetDataContext::AddChild(EggEditor2::IAsset const& /* asset */)
    {
        throw hresult_not_implemented();
    }
}
