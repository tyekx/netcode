#include "pch.h"
#include "UC_ProjectFolder.h"
#if __has_include("UC_ProjectFolder.g.cpp")
#include "UC_ProjectFolder.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggEditor2::implementation
{
    UC_ProjectFolder::UC_ProjectFolder()
    {
        InitializeComponent();
    }
}
