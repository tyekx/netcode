#include "pch.h"
#include "DC_MainPage.h"
#include "DC_MainPage.g.cpp"

namespace winrt::EggAssetEditor::implementation
{

    DC_MainPage::DC_MainPage() : meshes{ winrt::single_threaded_observable_vector<EggAssetEditor::DC_Mesh>() }, propertyChanged{} {

    }

    Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Mesh> DC_MainPage::Meshes()
    {
        return meshes;
    }

    void DC_MainPage::Meshes(Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Mesh> const& value)
    {
        meshes = value;
    }

    winrt::event_token DC_MainPage::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return propertyChanged.add(handler);
    }

    void DC_MainPage::PropertyChanged(winrt::event_token const& token) noexcept
    {
        propertyChanged.remove(token);
    }
}
