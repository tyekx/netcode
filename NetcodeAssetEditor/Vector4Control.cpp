#include "pch.h"
#include "Vector4Control.h"
#if __has_include("Vector4Control.g.cpp")
#include "Vector4Control.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggAssetEditor::implementation
{
    Vector4Control::Vector4Control()
    {
        InitializeComponent();

        auto dc = winrt::make<DC_Vector4Control>();

        wrapper().DataContext(dc);

        dc.PropertyChanged(
            Windows::UI::Xaml::Data::PropertyChangedEventHandler(this, &Vector4Control::OnValueUpdated)
        );
    }

    Windows::UI::Xaml::DependencyProperty Vector4Control::valueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"Value",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<EggAssetEditor::Vector4Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(Windows::Foundation::Numerics::float4{0.0f, 0.0f, 0.0f, 0.0f}), Windows::UI::Xaml::PropertyChangedCallback{ &Vector4Control::OnValueChanged } }
    );
}
