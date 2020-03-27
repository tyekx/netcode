#pragma once
#include "DC_Bone.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_Bone : DC_BoneT<DC_Bone>
    {
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
        hstring name;
        uint32_t depth;

        DC_Bone();

        uint32_t Depth();
        void Depth(uint32_t value);
        hstring Name();
        void Name(hstring const & value);
        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const & handler);
        void PropertyChanged(winrt::event_token const & token) noexcept;
    };
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_Bone : DC_BoneT<DC_Bone, implementation::DC_Bone>
    {
    };
}
