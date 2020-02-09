#include "pch.h"
#include "Vector3Control.h"
#if __has_include("Vector3Control.g.cpp")
#include "Vector3Control.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggAssetEditor::implementation
{
    Vector3Control::Vector3Control()
    {
        InitializeComponent();

        auto dc = winrt::make<DC_Vector3Control>();

        wrapper().DataContext(dc);

        dc.PropertyChanged(
            Windows::UI::Xaml::Data::PropertyChangedEventHandler(this, &Vector3Control::OnValueUpdated)
        );
    }

    Windows::UI::Xaml::DependencyProperty Vector3Control::valueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"Value",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<EggAssetEditor::Vector3Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(Windows::Foundation::Numerics::float3{0.0f, 0.0f, 0.0f}), Windows::UI::Xaml::PropertyChangedCallback{ &Vector3Control::OnValueChanged } }
    );
}
