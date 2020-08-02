#include "pch.h"
#include "DC_GeometryPage.h"
#include "DC_GeometryPage.g.cpp"

namespace winrt::NetcodeAssetEditor::implementation
{
    DC_GeometryPage::DC_GeometryPage() : DC_GeometryPageT<DC_GeometryPage>{},
        shared{}, uniformScale{}, rotationInDegrees{}, boundingBoxSize{}, propertyChanged{},
        transformBuffer{ winrt::single_threaded_observable_vector<Windows::Foundation::Numerics::float4x4>() } {

    }

    NetcodeAssetEditor::DC_MainPage DC_GeometryPage::Shared()
    {
        return shared;
    }
    void DC_GeometryPage::Shared(NetcodeAssetEditor::DC_MainPage const& value)
    {
        shared = value;
    }
    float DC_GeometryPage::UniformScale()
    {
        return uniformScale;
    }
    void DC_GeometryPage::UniformScale(float value)
    {
        uniformScale = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"UniformScale" });
    }

    Windows::Foundation::Numerics::float3 DC_GeometryPage::RotationInDegrees()
    {
        return rotationInDegrees;
    }

    void DC_GeometryPage::RotationInDegrees(Windows::Foundation::Numerics::float3 const& value)
    {
        rotationInDegrees = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"RotationInDegrees" });
    }

    Windows::Foundation::Numerics::float3 DC_GeometryPage::BoundingBoxSize()
    {
        return boundingBoxSize;
    }

    void DC_GeometryPage::BoundingBoxSize(Windows::Foundation::Numerics::float3 const & value)
    {
        boundingBoxSize = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ L"BoundingBoxSize" });
    }


    Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Numerics::float4x4> DC_GeometryPage::TransformBuffer()
    {
        return transformBuffer;
    }

    winrt::event_token DC_GeometryPage::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_GeometryPage::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }
}
