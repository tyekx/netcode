#include "pch.h"
#include "DC_Vector3Control.h"
#include "DC_Vector3Control.g.cpp"

namespace winrt::NetcodeAssetEditor::implementation
{
    Windows::Foundation::Numerics::float3 DC_Vector3Control::VectorValue()
    {
        return vectorValue;
    }
    void DC_Vector3Control::VectorValue(Windows::Foundation::Numerics::float3 const & value)
    {
        vectorValue = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueX"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueY"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueZ"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector3Control::ValueX()
    {
        return vectorValue.x;
    }

    void DC_Vector3Control::ValueX(float value)
    {
        vectorValue.x = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueX"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector3Control::ValueY()
    {
        return vectorValue.y;
    }

    void DC_Vector3Control::ValueY(float value)
    {
        vectorValue.y = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueY"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector3Control::ValueZ()
    {
        return vectorValue.z;
    }
    void DC_Vector3Control::ValueZ(float value)
    {
        vectorValue.z = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueZ"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }
    winrt::event_token DC_Vector3Control::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const & handler)
    {
        return propertyChanged.add(handler);
    }
    void DC_Vector3Control::PropertyChanged(winrt::event_token const & token) noexcept
    {
        propertyChanged.remove(token);
    }
}
