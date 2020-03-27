#include "pch.h"
#include "MaterialsPage.h"
#if __has_include("MaterialsPage.g.cpp")
#include "MaterialsPage.g.cpp"
#endif

#include "XamlGlobal.h"

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NetcodeAssetEditor::implementation
{
    MaterialsPage::MaterialsPage()
    {
        InitializeComponent();

        materials = winrt::single_threaded_observable_vector<NetcodeAssetEditor::DC_Material>();
    }

    Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Material> MaterialsPage::Materials() {
        return materials;
    }

    void MaterialsPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        materials.Clear();

        if(Global::Model != nullptr && !Global::Model->materials.empty()) {
            for(const auto & material : Global::Model->materials) {
                auto dcMat = winrt::make<NetcodeAssetEditor::implementation::DC_Material>();

                dcMat.Name(to_hstring(material.name));

                Windows::UI::Color color;
                color.A = static_cast<uint8_t>(material.diffuseColor.w * 255.0f);
                color.B = static_cast<uint8_t>(material.diffuseColor.z * 255.0f);
                color.G = static_cast<uint8_t>(material.diffuseColor.y * 255.0f);
                color.R = static_cast<uint8_t>(material.diffuseColor.x * 255.0f);

                dcMat.DiffuseColor(color);
                dcMat.FresnelR0(Windows::Foundation::Numerics::float3(material.fresnelR0.x, material.fresnelR0.y, material.fresnelR0.z));
                dcMat.AmbientMapReference(to_hstring(material.ambientMapReference));
                dcMat.DiffuseMapReference(to_hstring(material.diffuseMapReference));
                dcMat.HeightMapReference(L"");
                dcMat.NormalMapReference(to_hstring(material.normalMapReference));
                dcMat.RoughnessMapReference(to_hstring(material.roughnessMapReference));
                dcMat.SpecularMapReference(to_hstring(material.specularMapReference));

                materials.Append(dcMat);
            }
        }

        __super::OnNavigatedTo(e);
    }

    void MaterialsPage::PropertiesButton_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        int32_t selectedIndex = materialsList().SelectedIndex();

        if(selectedIndex < 0) {
            return;
        }

        auto materialsView = materials.GetView();

        if(materialsView.Size() <= selectedIndex ||
            Global::Model == nullptr ||
            Global::Model->materials.size() <= selectedIndex) {
            return;
        }

        auto selectedMaterial = materialsView.GetAt(selectedIndex);

        auto & materialData = Global::Model->materials.at(static_cast<size_t>(selectedIndex));

        materialData.diffuseMapReference = winrt::to_string(selectedMaterial.DiffuseMapReference());
        materialData.ambientMapReference = winrt::to_string(selectedMaterial.AmbientMapReference());
        materialData.normalMapReference = winrt::to_string(selectedMaterial.NormalMapReference());
        materialData.roughnessMapReference = winrt::to_string(selectedMaterial.RoughnessMapReference());
        materialData.specularMapReference = winrt::to_string(selectedMaterial.SpecularMapReference());
    }
}


