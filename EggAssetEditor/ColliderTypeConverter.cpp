#include "pch.h"
#include "ColliderTypeConverter.h"
#include "ColliderTypeConverter.g.cpp"
#include "Model.h"

namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::IInspectable ColliderTypeConverter::Convert(Windows::Foundation::IInspectable const& value, Windows::UI::Xaml::Interop::TypeName const& targetType, Windows::Foundation::IInspectable const& parameter, hstring const& language)
    {
        ColliderType type = static_cast<ColliderType>(unbox_value<uint32_t>(value));

        switch(type) {
            case ColliderType::BOX: 
                return box_value(L"Box");
            case ColliderType::CAPSULE:
                return box_value(L"Capsule");
            case ColliderType::MESH:
                return box_value(L"Mesh");
            case ColliderType::PLANE:
                return box_value(L"Plane");
            case ColliderType::SPHERE:
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
