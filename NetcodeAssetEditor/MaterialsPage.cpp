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


    void MaterialsPage::Shininess_TextChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::TextChangedEventArgs const & e)
    {
        int32_t matIdx = materialsList().SelectedIndex();

        if(matIdx < 0) {
            return;
        }

        auto floatBox = sender.try_as<NetcodeAssetEditor::FloatBox>();

        if(floatBox != nullptr) {
            Global::Model->materials[matIdx].shininess = std::clamp(floatBox.Value(), 0.0f, 256.0f);
            Global::EditorApp->ApplyMaterialData(matIdx, Global::Model->materials[matIdx]);
            Global::EditorApp->InvalidateFrame();
        }
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

                dcMat.Shininess(material.shininess);
                dcMat.DiffuseColor(color);
                dcMat.FresnelR0(Windows::Foundation::Numerics::float3(material.fresnelR0.x, material.fresnelR0.y, material.fresnelR0.z));
                dcMat.AmbientMapReference(material.ambientMapReference);
                dcMat.DiffuseMapReference(material.diffuseMapReference);
                dcMat.NormalMapReference(material.normalMapReference);
                dcMat.RoughnessMapReference(material.roughnessMapReference);
                dcMat.SpecularMapReference(material.specularMapReference);

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

        materialData.diffuseMapReference = selectedMaterial.DiffuseMapReference().c_str();
        materialData.ambientMapReference = selectedMaterial.AmbientMapReference().c_str();
        materialData.normalMapReference = selectedMaterial.NormalMapReference().c_str();
        materialData.roughnessMapReference = selectedMaterial.RoughnessMapReference().c_str();
        materialData.specularMapReference = selectedMaterial.SpecularMapReference().c_str();

        Global::EditorApp->ApplyMaterial(static_cast<uint32_t>(selectedIndex), materialData);
        Global::EditorApp->InvalidateFrame();
    }


    void MaterialsPage::ColorPicker_ColorChanged(Windows::UI::Xaml::Controls::ColorPicker const & sender, Windows::UI::Xaml::Controls::ColorChangedEventArgs const & args)
    {
        auto rgba = sender.Color();
        int32_t selectedIndex = materialsList().SelectedIndex();

        if(selectedIndex < 0) {
            return;
        }

        Netcode::Float4 color;
        color.x = static_cast<float>(rgba.R) / 255.0f;
        color.y = static_cast<float>(rgba.G) / 255.0f;
        color.z = static_cast<float>(rgba.B) / 255.0f;
        color.w = static_cast<float>(rgba.A) / 255.0f;

        if(Global::EditorApp != nullptr) {
            Global::Model->materials[selectedIndex].diffuseColor = color;
            Global::EditorApp->ApplyMaterialData(selectedIndex, Global::Model->materials[selectedIndex]);
            Global::EditorApp->InvalidateFrame();
        }
    }

    winrt::fire_and_forget MaterialsPage::TextBox_Drop(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::DragEventArgs const & e)
    {
        auto textBox = sender.try_as<Windows::UI::Xaml::Controls::TextBox>();

        if(textBox != nullptr) {
            if(e.DataView().Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems())) {
                auto deferral = e.GetDeferral();
                auto evtArgsCopy = e;

                auto storageItems = co_await e.DataView().GetStorageItemsAsync();

                if(storageItems.Size() == 1) {
                    
                    auto item = storageItems.GetAt(0);
                    std::wstring cpyRef = item.Path().c_str();

                    Netcode::IO::Path::UnifySlashes(cpyRef, Netcode::IO::Path::GetSlash());

                    if(Netcode::IO::Path::IsAbsolute(cpyRef)) {
                        auto mediaRoot = Netcode::IO::Path::MediaRoot();
                        if(cpyRef.find(mediaRoot) == 0) {
                            cpyRef.erase(0, mediaRoot.size());
                            textBox.Text(cpyRef);
                            evtArgsCopy.AcceptedOperation(Windows::ApplicationModel::DataTransfer::DataPackageOperation::Move);
                        }
                    }
                }

                deferral.Complete();
            }
        }
    }

    winrt::fire_and_forget MaterialsPage::TextBox_DragOver(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::DragEventArgs const & e) {
        auto textBox = sender.try_as<Windows::UI::Xaml::Controls::TextBox>();

        if(textBox != nullptr) {
            if(e.DataView().Contains(Windows::ApplicationModel::DataTransfer::StandardDataFormats::StorageItems())) {
                auto deferral = e.GetDeferral();
                auto evtArgsCopy = e;

                auto storageItems = co_await e.DataView().GetStorageItemsAsync();

                if(storageItems.Size() == 1) {
                    evtArgsCopy.AcceptedOperation(Windows::ApplicationModel::DataTransfer::DataPackageOperation::Move);
                }

                deferral.Complete();
            }
        }
    }
}


