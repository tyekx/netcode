#include "pch.h"
#include "DC_Material.h"
#include "DC_Material.g.cpp"

namespace winrt::NetcodeAssetEditor::implementation
{
    bool DC_Material::MetalMask() {
        return metalMask;
    }
    void DC_Material::MetalMask(bool value) {
        metalMask = value;
    }
    hstring DC_Material::Name()
    {
        return name;
    }

    void DC_Material::Name(hstring const& value)
    {
        name = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Name"));
    }

    Windows::UI::Color DC_Material::DiffuseColor()
    {
        return diffuseColor;
    }

    void DC_Material::DiffuseColor(Windows::UI::Color const& value)
    {
        diffuseColor = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"DiffuseColor"));
    }

    Windows::Foundation::Numerics::float3 DC_Material::FresnelR0()
    {
        return fresnelR0;
    }

    void DC_Material::FresnelR0(Windows::Foundation::Numerics::float3 const& value)
    {
        fresnelR0 = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"FresnelR0"));
    }

    Windows::Foundation::Numerics::float2 DC_Material::Tiling()
    {
        return tiling;
    }

    void DC_Material::Tiling(Windows::Foundation::Numerics::float2 const& value)
    {
        tiling = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Tiling"));
    }

    Windows::Foundation::Numerics::float2 DC_Material::TilingOffset()
    {
        return tilingOffset;
    }

    void DC_Material::TilingOffset(Windows::Foundation::Numerics::float2 const & value)
    {
        tilingOffset = value;
    }

    float DC_Material::Shininess()
    {
        return shininess;
    }

    void DC_Material::Shininess(float value)
    {
        shininess = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Shininess"));
    }

    float DC_Material::Reflectance()
    {
        return reflectance;
    }

    void DC_Material::Reflectance(float value)
    {
        reflectance = value;
    }

    float DC_Material::DisplacementScale() {
        return displacementScale;
    }

    void DC_Material::DisplacementScale(float value) {
        displacementScale = value;
    }

    float DC_Material::DisplacementBias()
    {
        return displacementBias;
    }

    void DC_Material::DisplacementBias(float value)
    {
        displacementBias = value;
    }

    hstring DC_Material::DiffuseMapReference()
    {
        return diffuseMapReference;
    }

    void DC_Material::DiffuseMapReference(hstring const& value)
    {
        diffuseMapReference = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"DiffuseMapReference"));
    }

    hstring DC_Material::NormalMapReference()
    {
        return normalMapReference;
    }

    void DC_Material::NormalMapReference(hstring const& value)
    {
        normalMapReference = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"NormalMapReference"));
    }

    hstring DC_Material::SpecularMapReference()
    {
        return specularMapReference;
    }

    void DC_Material::SpecularMapReference(hstring const& value)
    {
        specularMapReference = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"SpecularMapReference"));
    }

    hstring DC_Material::RoughnessMapReference()
    {
        return roughnessMapReference;
    }

    void DC_Material::RoughnessMapReference(hstring const& value)
    {
        roughnessMapReference = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"RoughnessMapReference"));
    }

    hstring DC_Material::AmbientMapReference()
    {
        return ambientMapReference;
    }

    void DC_Material::AmbientMapReference(hstring const& value)
    {
        ambientMapReference = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"AmbientMapReference"));
    }

    hstring DC_Material::DisplacementMapReference()
    {
        return displacementMapReference;
    }

    void DC_Material::DisplacementMapReference(hstring const& value)
    {
        displacementMapReference = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"DisplacementMapReference"));
    }

    winrt::event_token DC_Material::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_Material::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }
}
