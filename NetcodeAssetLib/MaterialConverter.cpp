#include "MaterialConverter.h"
#include <Netcode/Graphics/Material.h>
#include <Netcode/Utility.h>
#include "JsonUtility.h"

namespace Netcode {

    static void LoadParameters(Ptr<Netcode::Material> mat, const json11::Json::object & params) {
        switch(mat->GetType()) {
            case Netcode::MaterialType::BRDF: {
                mat->SetParameter(Netcode::MaterialParamId::DIFFUSE_ALBEDO, Netcode::Asset::LoadFloat4(params.find("diffuseAlbedo")->second));
                mat->SetParameter(Netcode::MaterialParamId::FRESNEL_R0, Netcode::Asset::LoadFloat3(params.find("fresnelR0")->second));
                mat->SetParameter(Netcode::MaterialParamId::ROUGHNESS, static_cast<float>(params.find("roughness")->second.number_value()));
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES, Netcode::Asset::LoadFloat2(params.find("textureTiles")->second));
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_TILES_OFFSET, Netcode::Asset::LoadFloat2(params.find("textureTilesOffset")->second));
                mat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_SCALE, static_cast<float>(params.find("displacementScale")->second.number_value()));
                mat->SetParameter(Netcode::MaterialParamId::DISPLACEMENT_BIAS, static_cast<float>(params.find("displacementBias")->second.number_value()));
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_DIFFUSE_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("diffusePath")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_NORMAL_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("normalPath")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_AMBIENT_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("ambientPath")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_SPECULAR_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("specularPath")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_ROUGHNESS_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("roughnessPath")->second.string_value()), Netcode::FullPathToken{} });
                mat->SetParameter(Netcode::MaterialParamId::TEXTURE_DISPLACEMENT_PATH, Netcode::URI::Texture{ Netcode::Utility::ToWideString(params.find("displacementPath")->second.string_value()), Netcode::FullPathToken{} });
            } break;

            default: break;
        }
    }

    static json11::Json::object StoreParameters(const Ref<Netcode::Material> & mat) {
        json11::Json::object convertedParams;

        switch(mat->GetType()) {
            case Netcode::MaterialType::BRDF: {
                convertedParams["diffuseAlbedo"] = Netcode::Asset::StoreFloat4(mat->GetRequiredParameter<Netcode::Float4>(Netcode::MaterialParamId::DIFFUSE_ALBEDO));
                convertedParams["fresnelR0"] = Netcode::Asset::StoreFloat3(mat->GetRequiredParameter<Netcode::Float3>(Netcode::MaterialParamId::FRESNEL_R0));
                convertedParams["roughness"] = mat->GetRequiredParameter<float>(Netcode::MaterialParamId::ROUGHNESS);
                convertedParams["textureTiles"] = Netcode::Asset::StoreFloat2(mat->GetRequiredParameter<Netcode::Float2>(Netcode::MaterialParamId::TEXTURE_TILES));
                convertedParams["textureTilesOffset"] = Netcode::Asset::StoreFloat2(mat->GetRequiredParameter<Netcode::Float2>(Netcode::MaterialParamId::TEXTURE_TILES_OFFSET));
                convertedParams["displacementScale"] = mat->GetRequiredParameter<float>(Netcode::MaterialParamId::DISPLACEMENT_SCALE);
                convertedParams["displacementBias"] = mat->GetRequiredParameter<float>(Netcode::MaterialParamId::DISPLACEMENT_BIAS);
                convertedParams["diffusePath"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_DIFFUSE_PATH).GetFullPath());
                convertedParams["normalPath"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_NORMAL_PATH).GetFullPath());
                convertedParams["ambientPath"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_AMBIENT_PATH).GetFullPath());
                convertedParams["specularPath"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_SPECULAR_PATH).GetFullPath());
                convertedParams["roughnessPath"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_ROUGHNESS_PATH).GetFullPath());
                convertedParams["displacementPath"] = Netcode::Utility::ToNarrowString(mat->GetRequiredParameter<Netcode::URI::Texture>(Netcode::MaterialParamId::TEXTURE_DISPLACEMENT_PATH).GetFullPath());
            } break;
            default: break;
        }

        return convertedParams;
    }

    std::vector<Netcode::Asset::Manifest::Material> ConvertToManifestMaterials(const std::vector<Ref<Netcode::Material>> & mats) {
        std::vector<Netcode::Asset::Manifest::Material> convertedMaterials;

        for(const auto & mat : mats) {
            Netcode::Asset::Manifest::Material m;
            m.name = mat->GetName();
            m.type = static_cast<int32_t>(mat->GetType());
            m.parameters = StoreParameters(mat);
            convertedMaterials.emplace_back(std::move(m));
        }

        return convertedMaterials;
    }

    std::vector<Ref<Netcode::Material>> ConvertToNetcodeMaterials(const std::vector<Netcode::Asset::Manifest::Material> & mats) {
        std::vector<Ref<Netcode::Material>> convertedMaterial;

        for(const auto & mat : mats) {
            Ref<Netcode::Material> m;

            switch(static_cast<Netcode::MaterialType>(mat.type)) {
                case Netcode::MaterialType::BRDF:
                    m = std::make_shared<Netcode::BrdfMaterial>(Netcode::MaterialType::BRDF, mat.name);
                    break;
                default: break;
            }

            LoadParameters(m.get(), mat.parameters);

            convertedMaterial.push_back(m);
        }

        return convertedMaterial;
    }

}
