#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "Vector4Control.g.h"
#include "DC_Vector4Control.h"
#include "FloatBox.h"

namespace winrt::EggAssetEditor::implementation
{
    struct Vector4Control : Vector4ControlT<Vector4Control>
    {
        Vector4Control();

        static Windows::UI::Xaml::DependencyProperty valueProperty;

        Windows::Foundation::IInspectable Value() {
            return GetValue(valueProperty);
        }

        static Windows::UI::Xaml::DependencyProperty ValueProperty() {
            return valueProperty;
        }

        void Value(Windows::Foundation::IInspectable const & value) {
            SetValue(valueProperty, value);
        }

        void OnValueUpdated(Windows::Foundation::IInspectable const & s, Windows::UI::Xaml::Data::PropertyChangedEventArgs const & e) {
            Value(s.as<EggAssetEditor::DC_Vector4Control>().VectorValue());
        }

        static void OnValueChanged(Windows::UI::Xaml::DependencyObject const & d, Windows::UI::Xaml::DependencyPropertyChangedEventArgs const & e)
        {
            auto f4n = unbox_value<Windows::Foundation::Numerics::float4>(e.NewValue());
            auto f4o = unbox_value<Windows::Foundation::Numerics::float4>(e.OldValue());

            if(f4n.x == f4o.x && f4n.y == f4o.y && f4n.z == f4o.z && f4n.w == f4o.w) {
                return;
            }

            auto iwlc{ d.as<EggAssetEditor::implementation::Vector4Control>() };

            iwlc->wrapper().DataContext().as<EggAssetEditor::DC_Vector4Control>().VectorValue(e.NewValue());
        }
    };
}

namespace winrt::EggAssetEditor::factory_implementation
{
    struct Vector4Control : Vector4ControlT<Vector4Control, implementation::Vector4Control>
    {
    };
}
