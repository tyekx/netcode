#pragma once
#include "DC_MainPage.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_MainPage : DC_MainPageT<DC_MainPage>
    {
        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Mesh> meshes;
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;

        DC_MainPage();

        void BoxedTestFloat4(Windows::Foundation::IInspectable const & value) {
            propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs(L"BoxedTestFloat4"));
        }

        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Mesh> Meshes();
        void Meshes(Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Mesh> const& value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept; 
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_MainPage : DC_MainPageT<DC_MainPage, implementation::DC_MainPage>
    {
    };
}
