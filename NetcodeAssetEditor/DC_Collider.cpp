#include "pch.h"
#include "DC_Collider.h"
#include "DC_Collider.g.cpp"

namespace winrt::NetcodeAssetEditor::implementation
{
    uint32_t DC_Collider::Type()
    {
        return static_cast<uint32_t>(type);
    }

    void DC_Collider::Type(uint32_t value)
    {
        type = static_cast<Netcode::Asset::ColliderType>( value);
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Type"));
    }

    int32_t DC_Collider::BoneReference()
    {
        return boneReference;
    }

    void DC_Collider::BoneReference(int32_t value)
    {
        boneReference = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"BoneReference"));
    }

    Windows::Foundation::Numerics::float3 DC_Collider::LocalPosition()
    {
        return localPosition;
    }

    void DC_Collider::LocalPosition(Windows::Foundation::Numerics::float3 const& value)
    {
        localPosition = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"LocalPosition"));
    }

    Windows::Foundation::Numerics::float3 DC_Collider::LocalRotation()
    {
        return localRotation;
    }

    void DC_Collider::LocalRotation(Windows::Foundation::Numerics::float3 const& value)
    {
        localRotation = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"LocalRotation"));
    }

    float DC_Collider::SphereArg()
    {
        return sphereArg;
    }

    void DC_Collider::SphereArg(float value)
    {
        sphereArg = value;
        Type(static_cast<uint32_t>(Netcode::Asset::ColliderType::SPHERE));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"SphereArg"));
    }

    Windows::Foundation::Numerics::float2 DC_Collider::CapsuleArgs()
    {
        return capsuleArgs;
    }

    void DC_Collider::CapsuleArgs(Windows::Foundation::Numerics::float2 const& value)
    {
        capsuleArgs = value;
        Type(static_cast<uint32_t>(Netcode::Asset::ColliderType::CAPSULE));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"CapsuleArgs"));
    }

    Windows::Foundation::Numerics::float3 DC_Collider::BoxArgs()
    {
        return boxArgs;
    }

    void DC_Collider::BoxArgs(Windows::Foundation::Numerics::float3 const& value)
    {
        boxArgs = value;
        Type(static_cast<uint32_t>(Netcode::Asset::ColliderType::BOX));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"BoxArgs"));
    }

    winrt::event_token DC_Collider::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_Collider::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }
}
