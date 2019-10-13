#pragma once
#include "AssetUIDataContext.g.h"

namespace winrt::EggEditor2::implementation
{
    struct AssetUIDataContext : AssetUIDataContextT<AssetUIDataContext>
    {
	private:
		Windows::UI::Color gradientStart;
		Windows::UI::Color gradientStop;
		Windows::UI::Xaml::Controls::Symbol assetIcon;
		int32_t editState;
		hstring overlayText;
		event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
	public:

        AssetUIDataContext() = default;

        Windows::UI::Color GradientStart();
        void GradientStart(Windows::UI::Color const& value);

        Windows::UI::Color GradientStop();
        void GradientStop(Windows::UI::Color const& value);

        hstring OverlayText();
        void OverlayText(hstring const& value);

        Windows::UI::Xaml::Controls::Symbol AssetIcon();
        void AssetIcon(Windows::UI::Xaml::Controls::Symbol const& value);

        int32_t EditState();
        void EditState(int32_t value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}

namespace winrt::EggEditor2::factory_implementation
{
	struct AssetUIDataContext : AssetUIDataContextT<AssetUIDataContext, implementation::AssetUIDataContext>
	{
	};
}
