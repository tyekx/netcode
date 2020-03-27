#pragma once

#include "DC_MainPage.h"
#include "UC_Mesh.h"
#include "GeometryPage.g.h"
#include "Model.h"
#include "EditorApp.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct GeometryPage : GeometryPageT<GeometryPage>
    {
        bool firstNavigation;

        GeometryPage();

        void ListView_SelectionChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::SelectionChangedEventArgs const & e);

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
