#include "pch.h"
#include "VectorConverter.h"
#include "VectorConverter.g.cpp"

namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::IInspectable VectorConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        try {
            auto f4 = unbox_value<Windows::Foundation::Numerics::float4>(value);

            return box_value(L"[" + to_hstring(f4.x) + L" ; " + to_hstring(f4.y) + L" ; " + to_hstring(f4.z) + L" ; " + to_hstring(f4.w) + L"]");
        } catch(winrt::hresult_no_interface& e) {

            auto f3 = unbox_value<Windows::Foundation::Numerics::float3>(value);

            return box_value(L"[" + to_hstring(f3.x) + L" ; " + to_hstring(f3.y) + L" ; " + to_hstring(f3.z) + L"]");
        }
    }

    Windows::Foundation::IInspectable VectorConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }
}
