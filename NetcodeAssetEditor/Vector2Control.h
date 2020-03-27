#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "Vector2Control.g.h"
#include "DC_Vector2Control.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct Vector2Control : Vector2ControlT<Vector2Control>
    {
        Vector2Control();

        static Windows::UI::Xaml::DependencyProperty valueProperty;

        Windows::Foundation::Numerics::float2 Value() {
            return unbox_value< Windows::Foundation::Numerics::float2>(GetValue(valueProperty));
        }

        static Windows::UI::Xaml::DependencyProperty ValueProperty() {
            return valueProperty;
        }

        void Value(Windows::Foundation::Numerics::float2 const & value) {
            SetValue(valueProperty, box_value(value));
        }

        void OnValueUpdated(Windows::Foundation::IInspectable const & s, Windows::UI::Xaml::Data::PropertyChangedEventArgs const & e) {
            Value(s.as<NetcodeAssetEditor::DC_Vector2Control>().VectorValue());
        }

        static void OnValueChanged(Windows::UI::Xaml::DependencyObject const & d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs const & e)
        {
            auto f4n = unbox_value<Windows::Foundation::Numerics::float2>(e.NewValue());
            auto f4o = unbox_value<Windows::Foundation::Numerics::float2>(e.OldValue());

            if(f4n.x == f4o.x && f4n.y == f4o.y) {
                return;
            }

            auto iwlc{ d.as<NetcodeAssetEditor::implementation::Vector2Control>() };

            iwlc->wrapper().DataContext().as<NetcodeAssetEditor::DC_Vector2Control>().VectorValue(f4n);
        }
    };
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct Vector2Control : Vector2ControlT<Vector2Control, implementation::Vector2Control>
    {
    };
}
