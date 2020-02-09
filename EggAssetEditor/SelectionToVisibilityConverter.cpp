#include "pch.h"
#include "SelectionToVisibilityConverter.h"
#include "SelectionToVisibilityConverter.g.cpp"


namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::IInspectable SelectionToVisibilityConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        int32_t idx = unbox_value<int32_t>(value);

        if(idx < 0) {
            return box_value(Windows::UI::Xaml::Visibility::Collapsed);
        } else {
            return box_value(Windows::UI::Xaml::Visibility::Visible);
        }
    }

    Windows::Foundation::IInspectable SelectionToVisibilityConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }
}
