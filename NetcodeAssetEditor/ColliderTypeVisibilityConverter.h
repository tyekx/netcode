#pragma once
#include "ColliderTypeVisibilityConverter.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct ColliderTypeVisibilityConverter : ColliderTypeVisibilityConverterT<ColliderTypeVisibilityConverter>
    {
        ColliderTypeVisibilityConverter() = default;

        Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
        Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct ColliderTypeVisibilityConverter : ColliderTypeVisibilityConverterT<ColliderTypeVisibilityConverter, implementation::ColliderTypeVisibilityConverter>
    {
    };
}
