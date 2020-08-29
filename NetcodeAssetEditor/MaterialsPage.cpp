#include "pch.h"
#include "MaterialsPage.h"
#if __has_include("MaterialsPage.g.cpp")
#include "MaterialsPage.g.cpp"
#endif

#include "XamlGlobal.h"
#include <Netcode/Utility.h>

using namespace winrt;
using namespace Windows::UI::Xaml;

namespace winrt::NetcodeAssetEditor::implementation
{
    MaterialsPage::MaterialsPage()
    {
        InitializeComponent();

        materials = winrt::single_threaded_observable_vector<NetcodeAssetEditor::DC_Material>();
    }

    static const wchar_t * GetTextureReference(bool isEnabled, const hstring & str) {
        return (isEnabled) ? str.c_str() : L"";
    }

    static void CopyTextureReference(Ptr<Netcode::Material> mat, int32_t idx, Netcode::MaterialParamId id, bool isChecked, const hstring & str) {
        std::wstring texRef = GetTextureReference(isChecked, str);
        Netcode::URI::Texture texUri;
        if(!texRef.empty()) {
            if(texRef.front() == L'/') {
                texUri = Netcode::URI::Texture{ std::move(texRef), Netcode::FullPathToken{} };
            } else {
                texUri = Netcode::URI::Texture{ std::move(texRef) };
            }
        }

        Netcode::MaterialParamId texId = static_cast<Netcode::MaterialParamId>(static_cast<uint32_t>(id)
            + (static_cast<uint32_t>(Netcode::MaterialParamId::TEXTURE_DIFFUSE) - static_cast<uint32_t>(Netcode::MaterialParamId::TEXTURE_DIFFUSE_PATH)));
        Global::EditorApp->ApplyTexture(idx, texId, id, texUri);
    }

    void MaterialsPage::UpdateMaterial(int32_t idx)
    {
        if(idx < 0) {
            return;
        }
        auto materialsView = materials.GetView();

        if(idx >= materialsView.Size()) {
            return;
        }

        auto selectedMaterial = materialsView.GetAt(idx);

        Ptr<Netcode::Material> mat = Global::Model->materials[idx].get();

        CopyTextureReference(mat, idx, Netcode::MaterialParamId::TEXTURE_DIFFUSE_PATH, diffuseTextureCheckBox().IsChecked().GetBoolean(), selectedMaterial.DiffuseMapReference());
        CopyTextureReference(mat, idx, Netcode::MaterialParamId::TEXTURE_NORMAL_PATH, normalTextureCheckBox().IsChecked().GetBoolean(), selectedMaterial.NormalMapReference());
        CopyTextureReference(mat, idx, Netcode::MaterialParamId::TEXTURE_AMBIENT_PATH, ambientTextureCheckBox().IsChecked().GetBoolean(), selectedMaterial.AmbientMapReference());
        CopyTextureReference(mat, idx, Netcode::MaterialParamId::TEXTURE_ROUGHNESS_PATH, roughnessTextureCheckBox().IsChecked().GetBoolean(), selectedMaterial.RoughnessMapReference());
        CopyTextureReference(mat, idx, Netcode::MaterialParamId::TEXTURE_SPECULAR_PATH, specularTextureCheckBox().IsChecked().GetBoolean(), selectedMaterial.SpecularMapReference());
        CopyTextureReference(mat, idx, Netcode::MaterialParamId::TEXTURE_DISPLACEMENT_PATH, displacementTextureCheckBox().IsChecked().GetBoolean(), selectedMaterial.DisplacementMapReference());

        Netcode::Float3 fresnelF0{ selectedMaterial.FresnelR0().x, selectedMaterial.FresnelR0().y, selectedMaterial.FresnelR0().z };
        Netcode::Float2 tilingOffset{ selectedMaterial.TilingOffset().x, selectedMaterial.TilingOffset().y };
        Netcode::Float2 tiling{ selectedMaterial.Tiling().x, selectedMaterial.Tiling().y };
        float displacementScale = selectedMaterial.DisplacementScale();
        float displacementBias = selectedMaterial.DisplacementBias();
        float reflectance = selectedMaterial.Reflectance();
        float roughness = std::clamp(1.0f - selectedMaterial.Shininess(), 0.0f, 1.0f);
        bool metalMask = selectedMaterial.MetalMask();

        mat->SetParameter(Netcode::MaterialParamId::METAL_MASK, metalMask);
        mat->SetParameter(Netcode::MaterialParamId::REFLECTANCE, reflectance);
        mat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES, tiling);
        mat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES_OFFSET, tilingOffset);
        mat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_SCALE, displacementScale);
        mat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_BIAS, displacementBias);
        mat->SetParameter(Netcode::MaterialParamId::ROUGHNESS, roughness);
        mat->SetParameter(Netcode::MaterialParamId::SPECULAR_ALBEDO, fresnelF0);

        Global::EditorApp->InvalidateFrame();
    }

    Windows::Foundation::Collections::IObservableVector<NetcodeAssetEditor::DC_Material> MaterialsPage::Materials() {
        return materials;
    }

    void MaterialsPage::Name_TextChanged(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::Controls::TextChangedEventArgs const & e)
    {
        UNREFERENCED_PARAMETER(e);

        auto textBox = sender.try_as<Windows::UI::Xaml::Controls::TextBox>();

        if(textBox != nullptr) {
            int32_t matIdx = materialsList().SelectedIndex();

            if(matIdx < 0) {
                return;
            }

            hstring text = textBox.Text();
            std::wstring wstr = text.c_str();

            Ptr<Netcode::Material> mat = Global::Model->materials[matIdx].get();

            mat->SetName(Netcode::Utility::ToNarrowString(wstr));
        }
    }

    void MaterialsPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs const & e) {
        materials.Clear();

        if(Global::Model != nullptr && !Global::Model->materials.empty()) {
            for(const Ref<Netcode::Material> & material : Global::Model->materials) {
                auto dcMat = winrt::make<NetcodeAssetEditor::implementation::DC_Material>();

                dcMat.Name(to_hstring(material->GetName()));

                using P = Netcode::MaterialParamId;

                Windows::UI::Color color;
                Netcode::Float4 diffuseColor = material->GetOptionalParameter<Netcode::Float4>(P::DIFFUSE_ALBEDO, Netcode::Float4::Zero);
                Netcode::Float2 tiling = material->GetOptionalParameter(P::TEXTURE_TILES, Netcode::Float2::One);
                Netcode::Float2 tilingOffset = material->GetOptionalParameter(P::TEXTURE_TILES_OFFSET, Netcode::Float2::Zero);
                Netcode::Float3 fresnelR0 = material->GetOptionalParameter(P::SPECULAR_ALBEDO, Netcode::Float3{ 0.05f, 0.05f, 0.05f});

                color.A = static_cast<uint8_t>(diffuseColor.w * 255.0f);
                color.B = static_cast<uint8_t>(diffuseColor.z * 255.0f);
                color.G = static_cast<uint8_t>(diffuseColor.y * 255.0f);
                color.R = static_cast<uint8_t>(diffuseColor.x * 255.0f);

                dcMat.MetalMask(material->GetOptionalParameter(P::METAL_MASK, false));
                dcMat.DisplacementBias(material->GetOptionalParameter(P::DISPLACEMENT_BIAS, 0.42f));
                dcMat.DisplacementScale(material->GetOptionalParameter(P::DISPLACEMENT_SCALE, 0.01f));
                dcMat.Shininess(1.0f - material->GetOptionalParameter(P::ROUGHNESS, 0.0f));
                dcMat.Reflectance(material->GetOptionalParameter(P::REFLECTANCE, 0.0f));
                dcMat.DiffuseColor(color);
                dcMat.Tiling(Windows::Foundation::Numerics::float2(tiling.x, tiling.y));
                dcMat.TilingOffset(Windows::Foundation::Numerics::float2());
                dcMat.FresnelR0(Windows::Foundation::Numerics::float3(fresnelR0.x, fresnelR0.y, fresnelR0.z));
                dcMat.AmbientMapReference(material->GetOptionalParameter<Netcode::URI::Texture>(P::TEXTURE_AMBIENT_PATH, Netcode::URI::Texture{}).GetFullPath());
                dcMat.DiffuseMapReference(material->GetOptionalParameter<Netcode::URI::Texture>(P::TEXTURE_DIFFUSE_PATH, Netcode::URI::Texture{}).GetFullPath());
                dcMat.NormalMapReference(material->GetOptionalParameter<Netcode::URI::Texture>(P::TEXTURE_NORMAL_PATH, Netcode::URI::Texture{}).GetFullPath());
                dcMat.RoughnessMapReference(material->GetOptionalParameter<Netcode::URI::Texture>(P::TEXTURE_ROUGHNESS_PATH, Netcode::URI::Texture{}).GetFullPath());
                dcMat.SpecularMapReference(material->GetOptionalParameter<Netcode::URI::Texture>(P::TEXTURE_SPECULAR_PATH, Netcode::URI::Texture{}).GetFullPath());
                dcMat.DisplacementMapReference(material->GetOptionalParameter<Netcode::URI::Texture>(P::TEXTURE_DISPLACEMENT_PATH, Netcode::URI::Texture{}).GetFullPath());

                materials.Append(dcMat);
            }
        }

        __super::OnNavigatedTo(e);
    }

    void MaterialsPage::PropertiesButton_Click(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e)
    {
        UpdateMaterial(materialsList().SelectedIndex());
    }

    void MaterialsPage::CheckBox_Checked(Windows::Foundation::IInspectable const & sender, Windows::UI::Xaml::RoutedEventArgs const & e) {
        UpdateMaterial(materialsList().SelectedIndex());
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
            Ptr<Netcode::Material> mat = Global::Model->materials[selectedIndex].get();

            mat->SetParameter(Netcode::MaterialParamId::DIFFUSE_ALBEDO, color);
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
