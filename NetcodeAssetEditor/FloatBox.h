#pragma once
#include "FloatBox.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct FloatBox : FloatBoxT<FloatBox>
    {
        FloatBox();

        float value;

        float TextToValue() {
            double v = _wtof(Text().c_str());

            return static_cast<float>(v);
        }

        static Windows::UI::Xaml::DependencyProperty minValueProperty;
        static Windows::UI::Xaml::DependencyProperty maxValueProperty;
        static Windows::UI::Xaml::DependencyProperty stepScaleProperty;
        double minValue;
        double maxValue;
        double stepScale;

        double StepScale() const {
            return unbox_value<double>(GetValue(stepScaleProperty));
        }

        void StepScale(double v) {
            stepScale = v;
            SetValue(stepScaleProperty, box_value(v));
        }

        static Windows::UI::Xaml::DependencyProperty StepScaleProperty() {
            return stepScaleProperty;
        }

        double MaxValue() const {
            return unbox_value<double>(GetValue(maxValueProperty));
        }

        void MaxValue(double v) {
            maxValue = v;
            SetValue(maxValueProperty, box_value(v));
        }

        static Windows::UI::Xaml::DependencyProperty MaxValueProperty() {
            return maxValueProperty;
        }

        double MinValue() const {
            return unbox_value<double>(GetValue(minValueProperty));
        }

        void MinValue(double v) {
            minValue = v;
            SetValue(minValueProperty, box_value(v));
        }

        static Windows::UI::Xaml::DependencyProperty MinValueProperty() {
            return minValueProperty;
        }

        float Value() const {
            return value;
        }

        void OnTextChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::TextChangedEventArgs const & e) {
            value = TextToValue();
        }

        void OnPointerWheelChanged(Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e) {
            __super::OnPointerWheelChanged(e);

            auto pointerPoint = e.GetCurrentPoint(*this);

            int32_t delta = pointerPoint.Properties().MouseWheelDelta();
            double sign = 0.0f;
            double speed = 1.0 * static_cast<float>(stepScale);

            uint32_t keyMods = static_cast<uint32_t>(e.KeyModifiers());
            uint32_t controlMod = static_cast<uint32_t>(Windows::System::VirtualKeyModifiers::Control);
            uint32_t shiftMod = static_cast<uint32_t>(Windows::System::VirtualKeyModifiers::Shift);

            if(keyMods == controlMod) {
                speed *= 0.1;
            }

            if(keyMods == shiftMod) {
                speed *= 5.0;
            }

            if(delta < 0) {
                sign = -1.0;
            }
            
            if(delta > 0) {
                sign = 1.0;
            }

            double vDouble = static_cast<double>(value) + sign * speed;

            value = static_cast<float>(std::clamp(vDouble, minValue, maxValue));
            Text(to_hstring(value));
            e.Handled(true);
        }
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct FloatBox : FloatBoxT<FloatBox, implementation::FloatBox>
    {
    };
}
