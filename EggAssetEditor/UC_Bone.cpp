#include "pch.h"
#include "UC_Bone.h"
#if __has_include("UC_Bone.g.cpp")
#include "UC_Bone.g.cpp"
#endif

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::EggAssetEditor::implementation
{
    UC_Bone::UC_Bone()
    {
        InitializeComponent();
    }
}
