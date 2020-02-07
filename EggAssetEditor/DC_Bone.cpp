#include "pch.h"
#include "DC_Bone.h"
#include "DC_Bone.g.cpp"

namespace winrt::EggAssetEditor::implementation
{

    DC_Bone::DC_Bone() : propertyChanged{}, name{} {

    }

    uint32_t DC_Bone::Depth()
    {
        return depth;
    }
    void DC_Bone::Depth(uint32_t value)
    {
        depth = value;
    }

    hstring DC_Bone::Name()
    {
        return name;
    }

    void DC_Bone::Name(hstring const& value)
    {
        name = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"Name"));
    }

    winrt::event_token DC_Bone::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_Bone::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }
}
