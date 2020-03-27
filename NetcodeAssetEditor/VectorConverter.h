#pragma once
#include "VectorConverter.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct VectorConverter : VectorConverterT<VectorConverter>
    {
        VectorConverter() = default;

        Windows::Foundation::IInspectable Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
        Windows::Foundation::IInspectable ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language);
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct VectorConverter : VectorConverterT<VectorConverter, implementation::VectorConverter>
    {
    };
}
