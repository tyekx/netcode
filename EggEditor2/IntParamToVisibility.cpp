#include "pch.h"
#include "IntParamToVisibility.h"
#include "IntParamToVisibility.g.cpp"

namespace winrt::EggEditor2::implementation
{
    Windows::Foundation::IInspectable IntParamToVisibility::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& /* targetType */, Windows::Foundation::IInspectable const& parameter, hstring const& /* language */ )
    {
		int32_t intValue = unbox_value<int32_t>(value);
		hstring param = unbox_value<hstring>(parameter);

		int32_t paramValue = _wtoi(param.c_str());

		return (intValue == paramValue) ? box_value(Windows::UI::Xaml::Visibility::Visible) : box_value(Windows::UI::Xaml::Visibility::Collapsed);
    }

    Windows::Foundation::IInspectable IntParamToVisibility::ConvertBack(Windows::Foundation::IInspectable const&, Windows::UI::Xaml::Interop::TypeName const& , Windows::Foundation::IInspectable const& , hstring const& )
    {
        throw hresult_not_implemented();
    }
}
