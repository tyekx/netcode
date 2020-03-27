#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "Vector3Control.g.h"
#include "DC_Vector3Control.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct Vector3Control : Vector3ControlT<Vector3Control>
    {
        Vector3Control();

        static Windows::UI::Xaml::DependencyProperty valueProperty;

       Windows::Foundation::Numerics::float3 Value() {
           return unbox_value< Windows::Foundation::Numerics::float3 >(GetValue(valueProperty));
        }

        static Windows::UI::Xaml::DependencyProperty ValueProperty() {
            return valueProperty;
        }

        void Value(Windows::Foundation::Numerics::float3 const & value) {
            SetValue(valueProperty, box_value(value));
        }

        void OnValueUpdated(Windows::Foundation::IInspectable const & s, Windows::UI::Xaml::Data::PropertyChangedEventArgs const & e) {
            Value(s.as<NetcodeAssetEditor::DC_Vector3Control>().VectorValue());
        }

        static void OnValueChanged(Windows::UI::Xaml::DependencyObject const & d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs const & e)
        {
            auto f4n = unbox_value<Windows::Foundation::Numerics::float3>(e.NewValue());
            auto f4o = unbox_value<Windows::Foundation::Numerics::float3>(e.OldValue());

            if(f4n.x == f4o.x && f4n.y == f4o.y && f4n.z == f4o.z ) {
                return;
            }

            auto iwlc{ d.as<NetcodeAssetEditor::implementation::Vector3Control>() };

            iwlc->wrapper().DataContext().as<NetcodeAssetEditor::DC_Vector3Control>().VectorValue(f4n);
        }
    };
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct Vector3Control : Vector3ControlT<Vector3Control, implementation::Vector3Control>
    {
    };
}
