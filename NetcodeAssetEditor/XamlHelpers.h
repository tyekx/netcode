#pragma once

#include <winrt/Windows.Foundation.h>
#include "XamlHelpers.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct XamlHelpers : XamlHelpersT<XamlHelpers>
    {
        XamlHelpers() = default;

        static Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Bone> ConvertBones();
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct XamlHelpers : XamlHelpersT<XamlHelpers, implementation::XamlHelpers>
    {
    };
}
