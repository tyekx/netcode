#include "pch.h"
#include "DC_Vector4Control.h"
#include "DC_Vector4Control.g.cpp"


namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::IInspectable DC_Vector4Control::VectorValue()
    {
        return box_value(vectorValue);
    }
    void DC_Vector4Control::VectorValue(Windows::Foundation::IInspectable const & value)
    {
        vectorValue = unbox_value<Windows::Foundation::Numerics::float4>(value);
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueX"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueY"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueZ"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueW"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector4Control::ValueX()
    {
        return vectorValue.x;
    }

    void DC_Vector4Control::ValueX(float value)
    {
        vectorValue.x = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueX"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector4Control::ValueY()
    {
        return vectorValue.y;
    }

    void DC_Vector4Control::ValueY(float value)
    {
        vectorValue.y = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueY"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector4Control::ValueZ()
    {
        return vectorValue.z;
    }
    void DC_Vector4Control::ValueZ(float value)
    {
        vectorValue.z = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueZ"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }
    float DC_Vector4Control::ValueW()
    {
        return vectorValue.w;
    }
    void DC_Vector4Control::ValueW(float value)
    {
        vectorValue.w = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueW"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }
    winrt::event_token DC_Vector4Control::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const & handler)
    {
        return propertyChanged.add(handler);
    }
    void DC_Vector4Control::PropertyChanged(winrt::event_token const & token) noexcept
    {
        propertyChanged.remove(token);
    }
}
