#pragma once

#include "MaterialsPage.g.h"
#include "DC_Material.h"
#include "VectorConverter.h"

namespace winrt::EggAssetEditor::implementation
{
    struct MaterialsPage : MaterialsPageT<MaterialsPage>
    {
        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Material> materials;

        MaterialsPage();

        Windows::Foundation::Collections::IObservableVector<EggAssetEditor::DC_Material> Materials();

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
		void PropertiesButton_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
	};
}

namespace winrt::EggAssetEditor::factory_implementation
{
    struct MaterialsPage : MaterialsPageT<MaterialsPage, implementation::MaterialsPage>
    {
    };
}
