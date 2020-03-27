#include "pch.h"
#include "DepthToMarginConverter.h"
#include "DepthToMarginConverter.g.cpp"

namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::IInspectable DepthToMarginConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        uint32_t depth = unbox_value<uint32_t>(value);
        uint32_t param = _wtoi(unbox_value<hstring>(parameter).c_str());

        Windows::UI::Xaml::Thickness thickness;
        thickness.Bottom = 0;
        thickness.Top = 0;
        thickness.Right = 0;
        thickness.Left = depth * param;
       
        return box_value(thickness);
    }
    Windows::Foundation::IInspectable DepthToMarginConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }
}
