#pragma once
#include "DC_Vector3Control.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_Vector3Control : DC_Vector3ControlT<DC_Vector3Control>
    {
        DC_Vector3Control() = default;

        winrt::event< Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        Windows::Foundation::Numerics::float3 vectorValue;

        Windows::Foundation::Numerics::float3 VectorValue();
        void VectorValue(Windows::Foundation::Numerics::float3 const& value);
        float ValueX();
        void ValueX(float value);
        float ValueY();
        void ValueY(float value);
        float ValueZ();
        void ValueZ(float value);
        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_Vector3Control : DC_Vector3ControlT<DC_Vector3Control, implementation::DC_Vector3Control>
    {
    };
}
