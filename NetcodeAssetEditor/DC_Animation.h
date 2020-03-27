#pragma once
#include "DC_Animation.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct DC_Animation : DC_AnimationT<DC_Animation>
    {
        float timescale;
        hstring name;
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        DC_Animation() = default;

        hstring Name();
        void Name(hstring const& value);

        float Timescale();
        void Timescale(float value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct DC_Animation : DC_AnimationT<DC_Animation, implementation::DC_Animation>
    {
    };
}
