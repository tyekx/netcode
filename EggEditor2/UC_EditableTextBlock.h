#pragma once

#include "winrt/Windows.UI.Xaml.h"
#include "winrt/Windows.UI.Xaml.Markup.h"
#include "winrt/Windows.UI.Xaml.Interop.h"
#include "winrt/Windows.UI.Xaml.Controls.Primitives.h"
#include "UC_EditableTextBlock.g.h"
#include "EditableTextBlockDataContext.h"

namespace winrt::EggEditor2::implementation
{
    struct UC_EditableTextBlock : UC_EditableTextBlockT<UC_EditableTextBlock>
    {
        UC_EditableTextBlock();

		hstring Value();

		void Value(hstring const & val);
    };
}

namespace winrt::EggEditor2::factory_implementation
{
    struct UC_EditableTextBlock : UC_EditableTextBlockT<UC_EditableTextBlock, implementation::UC_EditableTextBlock>
    {
    };
}
