#include "pch.h"
#include "AssetUIDataContext.h"
#include "AssetUIDataContext.g.cpp"

namespace winrt::EggEditor2::implementation
{
	Windows::UI::Color AssetUIDataContext::GradientStart()
	{
		return gradientStart;
	}

	void AssetUIDataContext::GradientStart(Windows::UI::Color const & value)
	{
		if(gradientStart != value) {
			gradientStart = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"GradientStart" });
		}
	}

	Windows::UI::Color AssetUIDataContext::GradientStop()
	{
		return gradientStop;
	}

	void AssetUIDataContext::GradientStop(Windows::UI::Color const & value)
	{
		if(gradientStop != value) {
			gradientStop = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"GradientStop" });
		}
	}

	hstring AssetUIDataContext::OverlayText()
	{
		return overlayText;
	}

	void AssetUIDataContext::OverlayText(hstring const & value)
	{
		if(overlayText != value) {
			overlayText = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"OverlayText" });
		}
	}

	Windows::UI::Xaml::Controls::Symbol AssetUIDataContext::AssetIcon()
	{
		return assetIcon;
	}

	void AssetUIDataContext::AssetIcon(Windows::UI::Xaml::Controls::Symbol const & value)
	{
		if(assetIcon != value) {
			assetIcon = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"AssetIcon" });
		}
	}

	int32_t AssetUIDataContext::EditState()
	{
		return editState;
	}

	void AssetUIDataContext::EditState(int32_t value)
	{
		if(editState != value) {
			editState = value;
			propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"EditState" });
		}
	}

	winrt::event_token AssetUIDataContext::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const & handler)
	{
		return propertyChanged.add(handler);
	}

	void AssetUIDataContext::PropertyChanged(winrt::event_token const & token) noexcept
	{
		propertyChanged.remove(token);
	}
}
