#include "pch.h"
#include "ColliderTypeVisibilityConverter.h"
#include "ColliderTypeVisibilityConverter.g.cpp"
#include "Model.h"


namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::IInspectable ColliderTypeVisibilityConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        uint32_t type = unbox_value<uint32_t>(value);

        hstring paramStr = unbox_value<hstring>(parameter);

        uint32_t paramInt = static_cast<uint32_t>(_wtoi(paramStr.c_str()));

        if(type == paramInt) {
            return box_value(Windows::UI::Xaml::Visibility::Visible);
        } else {
            return box_value(Windows::UI::Xaml::Visibility::Collapsed);
        }
    }

    Windows::Foundation::IInspectable ColliderTypeVisibilityConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }
}
