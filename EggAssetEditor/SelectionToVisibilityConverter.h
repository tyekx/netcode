#pragma once
#include "SelectionToVisibilityConverter.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct SelectionToVisibilityConverter : SelectionToVisibilityConverterT<SelectionToVisibilityConverter>
    {
        SelectionToVisibilityConverter() = default;

        Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
        Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct SelectionToVisibilityConverter : SelectionToVisibilityConverterT<SelectionToVisibilityConverter, implementation::SelectionToVisibilityConverter>
    {
    };
}
