#pragma once

#include <winrt/Windows.Foundation.h>
#include "XamlHelpers.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct XamlHelpers : XamlHelpersT<XamlHelpers>
    {
        XamlHelpers() = default;

        static Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Bone> ConvertBones();
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct XamlHelpers : XamlHelpersT<XamlHelpers, implementation::XamlHelpers>
    {
    };
}
