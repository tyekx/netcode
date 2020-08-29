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

        void UpdateMaterial(int32_t idx);

        Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Material> Materials();

        void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e);
		void PropertiesButton_Click(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        void ColorPicker_ColorChanged(winrt::Windows::UI::Xaml::Controls::ColorPicker const & sender, winrt::Windows::UI::Xaml::Controls::ColorChangedEventArgs const & args);
        winrt::fire_and_forget TextBox_Drop(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::DragEventArgs const & e);
        winrt::fire_and_forget TextBox_DragOver(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::DragEventArgs const & e);
        void CheckBox_Checked(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::RoutedEventArgs const & e);
        void Name_TextChanged(winrt::Windows::Foundation::IInspectable const & sender, winrt::Windows::UI::Xaml::Controls::TextChangedEventArgs const & e);
    };
}

namespace winrt::NetcodeAssetEditor::factory_implementation
{
    struct MaterialsPage : MaterialsPageT<MaterialsPage, implementation::MaterialsPage>
    {
    };
}
