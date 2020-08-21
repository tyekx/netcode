#include "pch.h"
#include "ColliderTypeConverter.h"
#include "ColliderTypeConverter.g.cpp"
#include <NetcodeAssetLib/Collider.h>

namespace winrt::NetcodeAssetEditor::implementation
{
    Windows::Foundation::IInspectable ColliderTypeConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        Netcode::Asset::ColliderType type = static_cast<Netcode::Asset::ColliderType>(unbox_value<uint32_t>(value));

        switch(type) {
            case Netcode::Asset::ColliderType::BOX:
                return box_value(L"Box");
            case Netcode::Asset::ColliderType::CAPSULE:
                return box_value(L"Capsule");
            case Netcode::Asset::ColliderType::MESH:
                return box_value(L"Mesh");
            case Netcode::Asset::ColliderType::PLANE:
                return box_value(L"Plane");
            case Netcode::Asset::ColliderType::SPHERE:
                return box_value(L"Sphere");
            default:
                return box_value(L"Undefined");
        }
    }

    Windows::Foundation::IInspectable ColliderTypeConverter::ConvertBack(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        throw hresult_not_implemented();
    }
}
