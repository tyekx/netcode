#include "pch.h"
#include "Vector4Control.h"
#if __has_include("Vector4Control.g.cpp")
#include "Vector4Control.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NetcodeAssetEditor::implementation
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

    Windows::UI::Xaml::DependencyProperty Vector4Control::isReadOnlyProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"IsReadOnly",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector4Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(false) }
    );

    Windows::UI::Xaml::DependencyProperty Vector4Control::valueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"Value",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector4Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(Windows::Foundation::Numerics::float4{0.0f, 0.0f, 0.0f, 0.0f}), Windows::UI::Xaml::PropertyChangedCallback{ &Vector4Control::OnValueChanged } }
    );

    Windows::UI::Xaml::DependencyProperty Vector4Control::minValueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"MinValue",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector4Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(-100000.0) }
    );

    Windows::UI::Xaml::DependencyProperty Vector4Control::maxValueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"MaxValue",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector4Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(100000.0) }
    );

    Windows::UI::Xaml::DependencyProperty Vector4Control::stepScaleProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"StepScale",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector3Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(1.0) }
    );
}
