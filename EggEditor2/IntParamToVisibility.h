#pragma once
#include "IntParamToVisibility.g.h"


namespace winrt::EggEditor2::implementation
{
    struct IntParamToVisibility : IntParamToVisibilityT<IntParamToVisibility>
    {
        IntParamToVisibility() = default;

        Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
        Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}
namespace winrt::EggEditor2::factory_implementation
{
    struct IntParamToVisibility : IntParamToVisibilityT<IntParamToVisibility, implementation::IntParamToVisibility>
    {
    };
}
