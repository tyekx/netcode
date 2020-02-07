#pragma once
#include "DC_MainPage.g.h"

namespace winrt::EggAssetEditor::implementation
{
    struct DC_MainPage : DC_MainPageT<DC_MainPage>
    {
        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Mesh> meshes;
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        DC_MainPage();

        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Mesh> Meshes();
        void Meshes(Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Mesh> const& value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept; 
    };
}
namespace winrt::EggAssetEditor::factory_implementation
{
    struct DC_MainPage : DC_MainPageT<DC_MainPage, implementation::DC_MainPage>
    {
    };
}
