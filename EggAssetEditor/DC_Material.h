#pragma once
#include "DC_Material.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct DC_Material : DC_MaterialT<DC_Material>
    {
        hstring name;
        Windows::Foundation::Numerics::float4 diffuseColor;
        Windows::Foundation::Numerics::float3 fresnelR0;

        float shininess;

        hstring diffuseMapReference;
        hstring normalMapReference;
        hstring specularMapReference;
        hstring roughnessMapReference;
        hstring ambientMapReference;
        hstring heightMapReference;

        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        DC_Material() = default;

        hstring Name();
        void Name(hstring const& value);

        Windows::Foundation::Numerics::float4 DiffuseColor();
        void DiffuseColor(Windows::Foundation::Numerics::float4 const& value);

        Windows::Foundation::Numerics::float3 FresnelR0();
        void FresnelR0(Windows::Foundation::Numerics::float3 const& value);

        float Shininess();
        void Shininess(float value);

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

        hstring HeightMapReference();
        void HeightMapReference(hstring const& value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct DC_Material : DC_MaterialT<DC_Material, implementation::DC_Material>
    {
    };
}
