#pragma once
#include "FloatBox.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct FloatBox : FloatBoxT<FloatBox>
    {
        FloatBox();

        float value;

        float TextToValue() {
            double v = _wtof(Text().c_str());

            return static_cast<float>(v);
        }

        void OnTextChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::TextChangedEventArgs const & e) {
            value = TextToValue();
        }

        void OnPointerWheelChanged(Windows::UI::Xaml::Input::PointerRoutedEventArgs const & e) {
            __super::OnPointerWheelChanged(e);

            auto pointerPoint = e.GetCurrentPoint(*this);

            int32_t delta = pointerPoint.Properties().MouseWheelDelta();
            float sign = 0.0f;
            float speed = 1.0f;

            uint32_t keyMods = static_cast<uint32_t>(e.KeyModifiers());
            uint32_t controlMod = static_cast<uint32_t>(Windows::System::VirtualKeyModifiers::Control);
            uint32_t shiftMod = static_cast<uint32_t>(Windows::System::VirtualKeyModifiers::Shift);

            if(keyMods == controlMod) {
                speed = 0.1f;
            }

            if(keyMods == shiftMod) {
                speed = 5.0f;
            }

            if(delta < 0) {
                sign = -1.0f;
            }
            
            if(delta > 0) {
                sign = 1.0f;
            }

            value = value + sign * speed;
            Text(to_hstring(value));
            e.Handled(true);
        }
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct FloatBox : FloatBoxT<FloatBox, implementation::FloatBox>
    {
    };
}
