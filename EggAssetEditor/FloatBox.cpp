#include "pch.h"
#include "FloatBox.h"
#include "FloatBox.g.cpp"

namespace winrt::EggAssetEditor::implementation
{
    FloatBox::FloatBox() {
        TextChanged(Windows::UI::Xaml::Controls::TextChangedEventHandler(this, &FloatBox::OnTextChanged));

        value = TextToValue();
    }
}
