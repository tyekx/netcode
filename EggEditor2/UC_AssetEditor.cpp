#include "pch.h"
#include "UC_AssetEditor.h"
#if __has_include("UC_AssetEditor.g.cpp")
#include "UC_AssetEditor.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggEditor2::implementation
{
    UC_AssetEditor::UC_AssetEditor()
    {
        InitializeComponent();
    }

    int32_t UC_AssetEditor::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void UC_AssetEditor::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void UC_AssetEditor::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        Button().Content(box_value(L"Clicked"));
    }
}
