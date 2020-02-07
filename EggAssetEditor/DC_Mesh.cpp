#include "pch.h"
#include "DC_Mesh.h"
#include "DC_Mesh.g.cpp"

namespace winrt::EggAssetEditor::implementation
{
    DC_Mesh::DC_Mesh() : lodLevels{ winrt::single_threaded_observable_vector<DC_Lod>() }, propertyChanged{} {

    }

    hstring DC_Mesh::Name()
    {
        return name;
    }
    void DC_Mesh::Name(hstring const & value)
    {
        name = value;
    }

    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Lod> DC_Mesh::LodLevels() {
        return lodLevels;
    }

    void DC_Mesh::LodLevels(Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Lod> const & value)
    {
        lodLevels = value;
        propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"LodLevels"));
    }

    winrt::event_token DC_Mesh::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_Mesh::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }
}
