#include "pch.h"
#include "DC_Animation.h"
#include "DC_Animation.g.cpp"

namespace winrt::NetcodeAssetEditor::implementation
{
    hstring DC_Animation::Name()
    {
        return name;
    }

    void DC_Animation::Name(hstring const& value)
    {
        name = value;
    }

    float DC_Animation::Timescale()
    {
        return timescale;
    }

    void DC_Animation::Timescale(float value)
    {
        timescale = value;
    }

    winrt::event_token DC_Animation::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_Animation::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }
}
