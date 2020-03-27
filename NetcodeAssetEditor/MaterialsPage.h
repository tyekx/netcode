#pragma once

#include "MaterialsPage.g.h"
#include "DC_Material.h"
#include "VectorConverter.h"

namespace winrt::NetcodeAssetEditor::implementation
{
    struct MaterialsPage : MaterialsPageT<MaterialsPage>
    {
        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Material> materials;

        MaterialsPage();

        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Material> Materials();

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
		void PropertiesButton_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
	};
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct MaterialsPage : MaterialsPageT<MaterialsPage, implementation::MaterialsPage>
    {
    };
}
