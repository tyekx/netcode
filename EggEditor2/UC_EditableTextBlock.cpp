#include "pch.h"
#include "UC_EditableTextBlock.h"
#if __has_include("UC_EditableTextBlock.g.cpp")
#include "UC_EditableTextBlock.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggEditor2::implementation
{
    UC_EditableTextBlock::UC_EditableTextBlock()
    {
        InitializeComponent();
    }

	hstring UC_EditableTextBlock::Value() {
		return TypedDataContext().Text();
	}

	void UC_EditableTextBlock::Value(hstring const & val) {
		TypedDataContext().Text(val);
	}
}
