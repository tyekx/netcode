#pragma once
#include "DC_Mesh.g.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct DC_Mesh : DC_MeshT<DC_Mesh>
    {
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler> propertyChanged;
        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Lod> lodLevels;
        hstring name;

        DC_Mesh();

        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Lod> LodLevels();
        void LodLevels(Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Lod> const& value);

        winrt::event_token PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        hstring Name();
        void Name(hstring const & value);
    };
}
namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct DC_Mesh : DC_MeshT<DC_Mesh, implementation::DC_Mesh>
    {
    };
}
