#include "pch.h"
#include "UC_Asset.h"
#if __has_include("UC_Asset.g.cpp")
#include "UC_Asset.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggEditor2::implementation
{
    UC_Asset::UC_Asset()
    {
        InitializeComponent();
    }

}
