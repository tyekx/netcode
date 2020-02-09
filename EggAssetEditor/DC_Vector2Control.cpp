#include "pch.h"
#include "DC_Vector2Control.h"
#include "DC_Vector2Control.g.cpp"

namespace winrt::EggAssetEditor::implementation
{
    Windows::Foundation::Numerics::float2 DC_Vector2Control::VectorValue()
    {
        return vectorValue;
    }
    void DC_Vector2Control::VectorValue(Windows::Foundation::Numerics::float2 const & value)
    {
        vectorValue = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueX"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueY"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector2Control::ValueX()
    {
        return vectorValue.x;
    }

    void DC_Vector2Control::ValueX(float value)
    {
        vectorValue.x = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueX"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    float DC_Vector2Control::ValueY()
    {
        return vectorValue.y;
    }

    void DC_Vector2Control::ValueY(float value)
    {
        vectorValue.y = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"ValueY"));
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"VectorValue"));
    }

    winrt::event_token DC_Vector2Control::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const & handler)
    {
        return propertyChanged.add(handler);
    }
    void DC_Vector2Control::PropertyChanged(winrt::event_token const & token) noexcept
    {
        propertyChanged.remove(token);
    }
}
