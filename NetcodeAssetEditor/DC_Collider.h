#pragma once
#include "DC_Collider.g.h"
#include <NetcodeAssetLib/Collider.h>

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_Collider : DC_ColliderT<DC_Collider>
    {
        DC_Collider() = default;

        union {
            Windows::Foundation::Numerics::float3 boxArgs;
            Windows::Foundation::Numerics::float2 capsuleArgs;
            float sphereArg;
        };

        Windows::Foundation::Numerics::float3 localPosition;
        Windows::Foundation::Numerics::float3 localRotation;

        Netcode::Asset::ColliderType type;
        int32_t boneReference;

        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;


        uint32_t Type();
        void Type(uint32_t value);

        int32_t BoneReference();
        void BoneReference(int32_t value);

        float SphereArg();
        void SphereArg(float value);

        Windows::Foundation::Numerics::float3 LocalPosition();
        void LocalPosition(Windows::Foundation::Numerics::float3 const& value);

        Windows::Foundation::Numerics::float3 LocalRotation();
        void LocalRotation(Windows::Foundation::Numerics::float3 const& value);

        Windows::Foundation::Numerics::float2 CapsuleArgs();
        void CapsuleArgs(Windows::Foundation::Numerics::float2 const& value);

        Windows::Foundation::Numerics::float3 BoxArgs();
        void BoxArgs(Windows::Foundation::Numerics::float3 const& value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_Collider : DC_ColliderT<DC_Collider, implementation::DC_Collider>
    {
    };
}
