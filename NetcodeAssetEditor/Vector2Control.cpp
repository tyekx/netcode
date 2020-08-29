#include "pch.h"
#include "Vector2Control.h"
#if __has_include("Vector2Control.g.cpp")
#include "Vector2Control.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NetcodeAssetEditor::implementation
{
    Vector2Control::Vector2Control()
    {
        InitializeComponent();

        auto dc = winrt::make<DC_Vector2Control>();

        wrapper().DataContext(dc);

        dc.PropertyChanged(
            Windows::UI::Xaml::Data::PropertyChangedEventHandler(this, &Vector2Control::OnValueUpdated)
        );
    }

    Windows::UI::Xaml::DependencyProperty Vector2Control::isReadOnlyProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"IsReadOnly",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector2Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(false) }
    );

    Windows::UI::Xaml::DependencyProperty Vector2Control::valueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"Value",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector2Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(Windows::Foundation::Numerics::float2{0.0f, 0.0f}), Windows::UI::Xaml::PropertyChangedCallback{ &Vector2Control::OnValueChanged } }
    );

    Windows::UI::Xaml::DependencyProperty Vector2Control::minValueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"MinValue",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector2Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(-100000.0) }
    );

    Windows::UI::Xaml::DependencyProperty Vector2Control::maxValueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"MaxValue",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector2Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(100000.0) }
    );

    Windows::UI::Xaml::DependencyProperty Vector2Control::stepScaleProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"StepScale",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::Vector2Control>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(1.0) }
    );

}
