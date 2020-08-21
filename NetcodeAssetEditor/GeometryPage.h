#pragma once

#include "DC_GeometryPage.h"
#include "UC_Mesh.h"
#include "GeometryPage.g.h"
#include <NetcodeAssetLib/IntermediateModel.h>
#include "EditorApp.h"
#include "GCommandTransform.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct GeometryPage : GeometryPageT<GeometryPage>
    {
        std::vector<Netcode::Intermediate::Mesh> transformedMeshes;
        bool firstNavigation;

        void UpdateRenderedModel();

        void OnModelChanged(uint64_t value);

        GeometryPage();

        void ListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e);

        void OnTransformBufferChanged(Windows::Foundation::Collections::IObservableVector<Windows::Foundation::Numerics::float4x4> sender, Windows::Foundation::Collections::IVectorChangedEventArgs args);

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
        void geometryListView_ContainerContentChanging(winrt::Windows::UI::Xaml::Controls::ListViewBase const & sender, winrt::Windows::UI::Xaml::Controls::ContainerContentChangingEventArgs const & args);
    };
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct GeometryPage : GeometryPageT<GeometryPage, implementation::GeometryPage>
    {
    };
}
