#pragma once
#include "DC_Vector2Control.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_Vector2Control : DC_Vector2ControlT<DC_Vector2Control>
    {
        DC_Vector2Control() = default;

        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        Windows::Foundation::Numerics::float2 vectorValue;

        Windows::Foundation::Numerics::float2 VectorValue();
        void VectorValue(Windows::Foundation::Numerics::float2 const & value);
        float ValueX();
        void ValueX(float value);
        float ValueY();
        void ValueY(float value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const & handler);
        void PropertyChanged(winrt::event_token const & token) noexcept;
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_Vector2Control : DC_Vector2ControlT<DC_Vector2Control, implementation::DC_Vector2Control>
    {
    };
}
