#pragma once
#include "DC_Vector4Control.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct DC_Vector4Control : DC_Vector4ControlT<DC_Vector4Control>
    {

        winrt::event< Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        Windows::Foundation::Numerics::float4 vectorValue;

        DC_Vector4Control() = default;

        Windows::Foundation::IInspectable VectorValue();
        void VectorValue(Windows::Foundation::IInspectable const& value);
        float ValueX();
        void ValueX(float value);
        float ValueY();
        void ValueY(float value);
        float ValueZ();
        void ValueZ(float value);
        float ValueW();
        void ValueW(float value);
        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct DC_Vector4Control : DC_Vector4ControlT<DC_Vector4Control, implementation::DC_Vector4Control>
    {
    };
}
