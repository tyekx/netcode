#pragma once
#include "DepthToMarginConverter.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct DepthToMarginConverter : DepthToMarginConverterT<DepthToMarginConverter>
    {
        DepthToMarginConverter() = default;

        Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
        Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct DepthToMarginConverter : DepthToMarginConverterT<DepthToMarginConverter, implementation::DepthToMarginConverter>
    {
    };
}
