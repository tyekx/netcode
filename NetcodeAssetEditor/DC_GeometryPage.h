#pragma once
#include "DC_GeometryPage.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_GeometryPage : DC_GeometryPageT<DC_GeometryPage>
    {
        NetcodeAssetEditor::DC_MainPage shared;
        float uniformScale;
        Windows::Foundation::Numerics::float3 rotationInDegrees;
        Windows::Foundation::Numerics::float3 boundingBoxSize;
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Numerics::float4x4> transformBuffer;

        DC_GeometryPage();

        NetcodeAssetEditor::DC_MainPage Shared();
        void Shared(NetcodeAssetEditor::DC_MainPage const& value);

        float UniformScale();
        void UniformScale(float value);

        Windows::Foundation::Numerics::float3 RotationInDegrees();
        void RotationInDegrees(Windows::Foundation::Numerics::float3 const& value);

        Windows::Foundation::Numerics::float3 BoundingBoxSize();
        void BoundingBoxSize(Windows::Foundation::Numerics::float3 const & value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Numerics::float4x4> TransformBuffer();
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_GeometryPage : DC_GeometryPageT<DC_GeometryPage, implementation::DC_GeometryPage>
    {
    };
}
