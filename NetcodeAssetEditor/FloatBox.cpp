#include "pch.h"
#include "FloatBox.h"
#include "FloatBox.g.cpp"

namespace winrt::NetcodeAssetEditor::implementation
{
    FloatBox::FloatBox() {
        TextChanged(Windows::UI::Xaml::Controls::TextChangedEventHandler(this, &FloatBox::OnTextChanged));

        value = TextToValue();
    }

    Windows::UI::Xaml::DependencyProperty FloatBox::minValueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"MinValue",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::FloatBox>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(-100000.0) }
    );

    Windows::UI::Xaml::DependencyProperty FloatBox::maxValueProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"MaxValue",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::FloatBox>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(100000.0) }
    );

    Windows::UI::Xaml::DependencyProperty FloatBox::stepScaleProperty = Windows::UI::Xaml::DependencyProperty::Register(
        L"StepScale",
        winrt::xaml_typename<Windows::Foundation::IInspectable>(),
        winrt::xaml_typename<NetcodeAssetEditor::FloatBox>(),
        Windows::UI::Xaml::PropertyMetadata{ box_value(1.0) }
    );
}
