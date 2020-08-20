#pragma once
#include "DC_Material.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_Material : DC_MaterialT<DC_Material>
    {
        hstring name;
        Windows::UI::Color diffuseColor;
        Windows::Foundation::Numerics::float3 fresnelR0;
        Windows::Foundation::Numerics::float2 tiling;

        float shininess;
        float displacementScale;

        hstring diffuseMapReference;
        hstring normalMapReference;
        hstring specularMapReference;
        hstring roughnessMapReference;
        hstring ambientMapReference;
        hstring displacementMapReference;

        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        DC_Material() = default;

        hstring Name();
        void Name(hstring const& value);

        Windows::UI::Color DiffuseColor();
        void DiffuseColor(Windows::UI::Color const& value);

        Windows::Foundation::Numerics::float3 FresnelR0();
        void FresnelR0(Windows::Foundation::Numerics::float3 const& value);

        Windows::Foundation::Numerics::float2 Tiling();
        void Tiling(Windows::Foundation::Numerics::float2 const& value);

        float Shininess();
        void Shininess(float value);

        float DisplacementScale();
        void DisplacementScale(float value);

        hstring DiffuseMapReference();
        void DiffuseMapReference(hstring const& value);

        hstring NormalMapReference();
        void NormalMapReference(hstring const& value);

        hstring SpecularMapReference();
        void SpecularMapReference(hstring const& value);

        hstring RoughnessMapReference();
        void RoughnessMapReference(hstring const& value);

        hstring AmbientMapReference();
        void AmbientMapReference(hstring const& value);

        hstring DisplacementMapReference();
        void DisplacementMapReference(hstring const& value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_Material : DC_MaterialT<DC_Material, implementation::DC_Material>
    {
    };
}
